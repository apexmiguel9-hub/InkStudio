package org.inkscape.android

import android.app.Application
import android.content.Context
import android.util.Log

class InkStudioApplication : Application() {

    companion object {
        private const val TAG = "InkStudio"
        @Volatile private var sNativeLoaded = false
    }

    override fun onCreate() {
        super.onCreate()
        // Cargar librerías nativas en orden de dependencias
        loadNativeLibraries()
    }

    private fun loadNativeLibraries() {
        if (sNativeLoaded) return

        // Orden CRÍTICO: dependencias primero
        val libs = listOf(
            "c++_shared",          // libc++_shared.so (si no usamos c++_static)
            "z",                   // libz.so
            "xml2",                // libxml2.so
            "png16",               // libpng16.so
            "jpeg",                // libjpeg.so
            "freetype",            // libfreetype.so
            "harfbuzz",            // libharfbuzz.so
            "fribidi",             // libfribidi.so
            "pixman-1",            // libpixman-1.so
            "epoxy",               // libepoxy.so
            "graphene-1.0",        // libgraphene-1.0.so
            "gobject-2.0",         // libgobject-2.0.so
            "glib-2.0",            // libglib-2.0.so
            "gio-2.0",             // libgio-2.0.so
            "gmodule-2.0",         // libgmodule-2.0.so
            "pcre2-8",             // libpcre2-8.so
            "ffi",                 // libffi.so
            "intl",                // libintl.so (gettext)
            "iconv",               // libiconv.so
            "lcms2",               // liblcms2.so
            "brotlidec",           // libbrotlidec.so
            "brotlicommon",        // libbrotlicommon.so
            "expat",               // libexpat.so
            "zstd",                // libzstd.so
            "cairo",               // libcairo.so
            "cairo-gobject",       // libcairo-gobject.so
            "pango-1.0",           // libpango-1.0.so
            "pangocairo-1.0",      // libpangocairo-1.0.so
            "pangoft2-1.0",        // libpangoft2-1.0.so
            "gdk_pixbuf-2.0",      // libgdk_pixbuf-2.0.so
            "gtk-4",               // libgtk-4.so (shared, 28MB)
            "sigc-3.0",            // libsigc-3.0.so
            "glibmm-2.68",         // libglibmm-2.68.so
            "giomm-2.68",          // libgiomm-2.68.so
            "cairomm-1.16",        // libcairomm-1.16.so
            "pangomm-2.48",        // libpangomm-2.48.so
            "gtkmm-4.0",           // libgtkmm-4.0.so
            "inkscape",            // libinkscape.so (nuestra lib principal)
            "inkview",             // libinkview.so
            "inkscape_base",       // libinkscape_base.so
            "2geom",               // lib2geom.so
            "depixelize",          // libdepixelize.so
            "avoid",               // libavoid.so
            "cola",                // libcola.so
            "vpsc",                // libvpsc.so
            "croco",               // libcroco.so
            "uemf",                // libuemf.so
            "xslt",                // libxslt.so
            "boost_stacktrace_basic", // libboost_stacktrace_basic.so
        )

        for (lib in libs) {
            try {
                System.loadLibrary(lib)
                Log.d(TAG, "Loaded: $lib")
            } catch (e: UnsatisfiedLinkError) {
                Log.w(TAG, "Optional lib not found (may be linked statically): $lib")
            }
        }

        sNativeLoaded = true
        Log.i(TAG, "All native libraries loaded")
    }

    companion object {
        fun isNativeLoaded(): Boolean = sNativeLoaded
    }
}