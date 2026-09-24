package org.inkscape.android

import android.app.Application
import android.content.Context
import android.util.Log
import org.gtk.android.RuntimeApplication

/**
 * Application class que usa el flujo oficial de GTK para Android.
 * Extiende RuntimeApplication que:
 * - Carga libgtk-4.so en bloque estático
 * - En onCreate() escribe recursos del sistema y llama startRuntime("inkscape")
 * - startRuntime() corre en hilo GTK y llama al main() nativo de libinkscape.so
 */
class InkStudioApplication : RuntimeApplication() {

    companion object {
        private const val TAG = "InkStudio"
    }

    override fun onCreate() {
        super.onCreate()
        // RuntimeApplication.onCreate() ya hace TODO el flujo oficial GTK Android:
        //   1. SystemFilesystem.writeResources(this) -> copia assets a filesDir
        //   2. startRuntime("inkscape") -> hilo GTK -> main() real de libinkscape.so
        // GTK Android runtime (gdk_android_runtime.c) ya configura HOME/XDG
        // vía g_set_user_dirs() desde getFilesDir(). No hardcodear rutas.
        Log.i(TAG, "Inkscape GTK runtime iniciado (flujo oficial)")
    }
}