#!/usr/bin/env python3
"""
reorder_init_array.py — Fix SIOF (Static Init Order Fiasco) en libinkscape_base.so

Problema: en Android, glibmm/gtkmm/sigc++/glib se enlazan ESTATICAMENTE dentro de
libinkscape_base.so. El `.init_array` ejecuta los constructores estáticos en orden
de enlace: los ctors de Inkscape (p.ej. InkscapePreferences::ModelColumns) corren
ANTES que los ctors de glibmm (p.ej. los std::map de value_custom.cc) -> uso de
mapas sin construir -> SIGSEGV `ldr x9,[x9]` al hacer dlopen.

En desktop, glibmm es una librería compartida que el linker inicializa ANTES que
libinkscape_base.so, por eso nunca se ve.

Solución: reordenar `.init_array` con una stable-partition:
  - grupo FRONT: ctors que SOLO referencian símbolos de dependencias
                (glib, glibmm, gtkmm, sigc, libc++, etc.) -> se ejecutan primero
  - grupo LATE : ctors que referencian cualquier símbolo de Inkscape
                -> se ejecutan después
Preservando el orden relativo DENTRO de cada grupo.

Seguridad: los slots de `.init_array` se reescriben con los MISMOS valores
(permutación), en las MISMAS posiciones de archivo. El loader aplica la misma
relocación (RELR/RELATIVE: contenido + load_bias) a cada slot, así que el
resultado es correcto. No se tocan secciones ni relocaciones.

Uso:
  python3 reorder_init_array.py <lib.so> [objdump_bin]
  - objdump_bin: objdump aarch64 (GNU u LLVM). Si se omite, autodetecta.

Salida: reescribe <lib.so> in-place (solo .init_array) y devuelve 0.
"""

import os
import re
import struct
import subprocess
import sys
import tempfile

# ---------------------------------------------------------------------------
# Parsing ELF64 mínimo (stdlib puro)
# ---------------------------------------------------------------------------

def read_cstr(data, off):
    end = data.index(b"\x00", off)
    return data[off:end].decode("utf-8", "replace")


def elf_sections(path):
    with open(path, "rb") as f:
        data = f.read()
    if data[:4] != b"\x7fELF":
        raise SystemExit(f"{path}: no es un ELF")
    if data[4] != 2:
        raise SystemExit(f"{path}: solo se soporta ELF64")
    if data[5] != 1:
        raise SystemExit(f"{path}: solo little-endian")

    e_shoff = struct.unpack_from("<Q", data, 0x28)[0]
    e_shentsize = struct.unpack_from("<H", data, 0x3A)[0]
    e_shnum = struct.unpack_from("<H", data, 0x3C)[0]
    e_shstrndx = struct.unpack_from("<H", data, 0x3E)[0]

    headers = []
    for i in range(e_shnum):
        off = e_shoff + i * e_shentsize
        sh = struct.unpack_from("<IIQQQQIIQQ", data, off)
        headers.append({
            "name": sh[0], "type": sh[1], "flags": sh[2],
            "addr": sh[3], "offset": sh[4], "size": sh[5],
            "link": sh[6], "info": sh[7], "align": sh[8], "entsize": sh[9],
        })

    shstr = headers[e_shstrndx]
    shstr_data = data[shstr["offset"]:shstr["offset"] + shstr["size"]]
    for h in headers:
        h["name"] = read_cstr(shstr_data, h["name"])

    secs = {}
    for h in headers:
        secs.setdefault(h["name"], h)
    return data, secs


def parse_dynsym(data, secs):
    sym = secs.get(".dynsym")
    if not sym:
        return []
    strtab = secs.get(".dynstr")
    strdata = data[strtab["offset"]:strtab["offset"] + strtab["size"]]
    n = sym["size"] // 24
    syms = []
    for i in range(n):
        off = sym["offset"] + i * 24
        st_name, st_info, st_other, st_shndx, st_value, st_size = \
            struct.unpack_from("<IBBHQQ", data, off)
        if st_info & 0xF == 2:  # STT_FUNC
            name = read_cstr(strdata, st_name) if st_name else ""
            syms.append((st_value, st_size, name))
    return syms


def nearest_symbol(syms, addr):
    best = None
    best_name = None
    for st_value, st_size, name in syms:
        if st_value <= addr and (best is None or st_value > best):
            best = st_value
            best_name = name
    return best_name


# ---------------------------------------------------------------------------
# Desensamblado único + parse lineal
# ---------------------------------------------------------------------------

INKSCAPE_RE = re.compile(r"Inkscape")
BL_RE = re.compile(r"\bbl\b")
TSO_RE = re.compile(r"^\s*([0-9a-f]+):\s+(.*)$")
HEX_TGT_RE = re.compile(r"(?:0x)?([0-9a-f]{6,})")


def find_objdump(hint):
    if hint:
        return hint
    for cand in ("aarch64-linux-gnu-objdump", "llvm-objdump", "objdump"):
        for p in ("/usr/bin/", "/usr/local/bin/", "/opt/", ""):
            full = p + cand
            if os.path.exists(full):
                return full
    raise SystemExit("ERROR: no se encontró objdump aarch64; pásalo como arg[2]")


def dump_text(objdump, lib):
    """Un solo objdump -d sobre todo el .text. Devuelve lista de líneas."""
    r = subprocess.run([objdump, "-d", lib], capture_output=True, text=True)
    if r.returncode != 0:
        raise SystemExit(f"objdump falló: {r.stderr[-500:]}")
    return r.stdout.splitlines()


def collect_bodies(lines, entries):
    """
    Un único pase lineal: para cada .init_array entry (por dirección), junta las
    líneas de su función hasta el primer `ret`. entries: lista de direcciones.
    Devuelve dict addr -> [líneas] y dict addr -> flags.
    """
    entry_set = set(entries)
    entry_sorted = sorted(entries)
    ptr = 0
    n = len(entry_sorted)
    bodies = {}
    current = None      # entrada activa
    done = False        # si ya vimos ret de la actual
    body = []

    for line in lines:
        m = TSO_RE.match(line)
        if not m:
            continue
        addr = int(m.group(1), 16)
        text = m.group(2)

        # ¿esta línea es el inicio de una nueva entry?
        if addr in entry_set and current != addr:
            if current is not None:
                bodies[current] = body
            current = addr
            body = [line]
            done = False
            continue

        if current is not None and not done:
            # terminar en el primer ret de la función
            if re.search(r"\bret$|\bret\b", text):
                body.append(line)
                bodies[current] = body
                done = True
                current = None
                body = []
            else:
                body.append(line)

        # avanzar ptr (por si el dump empieza en mitad; no necesario aquí)
        while ptr < n and entry_sorted[ptr] < addr:
            ptr += 1

    if current is not None and body:
        bodies[current] = body
    return bodies


def classify_body(text, syms):
    """
    LATE si la función referencia símbolos de Inkscape.
    Señales: anotaciones <...> del disassembler en líneas `bl` y `adrp`
    (contienen el nombre del símbolo, incluido @@Base/+off), más resolución
    por nearest-symbol para calls sin anotación.
    """
    for line in text.splitlines():
        if BL_RE.search(line):
            for name in re.findall(r"<([^>]+)>", line):
                base = name.split("@@")[0].split("@")[0]
                if INKSCAPE_RE.search(base):
                    return True
            # call sin anotación -> nearest-symbol
            m = TSO_RE.match(line)
            if m:
                tgt = HEX_TGT_RE.search(m.group(2))
                if tgt:
                    val = int(tgt.group(1), 16)
                    nm = nearest_symbol(syms, val)
                    if nm and INKSCAPE_RE.search(nm):
                        return True
        elif re.search(r"\badrp\b", line):
            for name in re.findall(r"<([^>]+)>", line):
                base = name.split("@@")[0].split("@")[0]
                if INKSCAPE_RE.search(base):
                    return True
    return False


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    if len(sys.argv) < 2:
        raise SystemExit(__doc__)
    lib = sys.argv[1]
    objdump = find_objdump(sys.argv[2] if len(sys.argv) > 2 else None)

    data, secs = elf_sections(lib)
    if ".init_array" not in secs:
        raise SystemExit(f"{lib}: sin .init_array")
    iarr = secs[".init_array"]
    if iarr["size"] == 0 or iarr["size"] % 8 != 0:
        raise SystemExit(f"{lib}: .init_array tamaño inválido {iarr['size']}")
    n = iarr["size"] // 8
    entries = list(struct.unpack_from("<%dQ" % n, data, iarr["offset"]))
    syms = parse_dynsym(data, secs)

    print(f"lib        : {lib}")
    print(f".init_array: {n} entradas @ 0x{iarr['addr']:x} (file 0x{iarr['offset']:x})")
    print(f"dynsym FUNC: {len(syms)}")

    print("Desensamblando .text (una sola pasada)...")
    lines = dump_text(objdump, lib)
    print(f"  {len(lines)} líneas")

    real = [e for e in entries if e != 0]
    bodies = collect_bodies(lines, real)
    missing = [hex(e) for e in real if e not in bodies]
    print(f"  cuerpos capturados: {len(bodies)}/{len(real)}"
          + (f"  (FALTAN: {missing})" if missing else ""))

    front, late = [], []
    for e in entries:
        if e == 0:
            front.append(e)
            continue
        body = bodies.get(e)
        if body is None:
            print(f"  WARN: sin cuerpo para 0x{e:x}; se deja en FRONT")
            front.append(e)
            continue
        text = "\n".join(body)
        if classify_body(text, syms):
            late.append(e)
        else:
            front.append(e)

    print(f"FRONT (deps) : {len(front)}")
    print(f"LATE (Inkscape): {len(late)}")
    if not late:
        print("Sin ctors de Inkscape detectados -> nada que mover (OK).")
        return 0

    # anclas de auditoría: ctors que tocan funcmap/ModelColumns
    mdl = re.compile(r"ModelColumns|custom_boxed_type_cpp_register")
    anchors_pre, anchors_post = [], []
    for i, e in enumerate(entries):
        body = bodies.get(e)
        if body and mdl.search("\n".join(body)):
            anchors_pre.append((i, e))

    new_entries = front + late
    assert sorted(new_entries) == sorted(entries), "¡se perdieron entradas!"

    # verificar que los anchos son iguales (mismo set de valores)
    assert set(new_entries) == set(entries)

    with open(lib, "r+b") as f:
        f.seek(iarr["offset"])
        f.write(struct.pack("<%dQ" % n, *new_entries))
        f.flush()
        os.fsync(f.fileno())

    for i, e in enumerate(new_entries):
        body = bodies.get(e)
        if body and mdl.search("\n".join(body)):
            anchors_post.append((i, e))

    print("\nAnclas (modelcolumns/funcmap ctors):")
    for lab, arr in (("ANTES ", anchors_pre), ("DESPUES", anchors_post)):
        for i, e in arr:
            grp = "FRONT" if i < len(front) else "LATE"
            print(f"  {lab}: idx={i:3d} addr=0x{e:x} [{grp}]")
    print("\nOK: .init_array reordenado (deps antes de Inkscape).")
    return 0


if __name__ == "__main__":
    sys.exit(main())