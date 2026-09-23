package org.inkscape.android

import android.app.Application
import android.content.Context
import android.util.Log

class InkStudioApplication : Application() {

    companion object {
        private const val TAG = "InkStudio"
        @Volatile private var sNativeLoaded = false

        fun isNativeLoaded(): Boolean = sNativeLoaded
    }

    override fun onCreate() {
        super.onCreate()
        // Cargar librerías nativas en orden de dependencias
        loadNativeLibraries()
    }

    private fun loadNativeLibraries() {
        if (sNativeLoaded) return

        // SOLO libs que EXISTEN como .so en jniLibs/
        // El resto (cairo, pango, glib, harfbuzz, freetype, etc.) se linkean ESTÁTICAMENTE
        // dentro de libgtk-4.so (28MB) y libinkscape.so
        val libs = listOf(
            "gtk-4",               // libgtk-4.so (shared, 28MB) - DEBE estar
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

        var loaded = 0
        for (lib in libs) {
            try {
                System.loadLibrary(lib)
                Log.d(TAG, "Loaded: $lib")
                loaded++
            } catch (e: UnsatisfiedLinkError) {
                Log.w(TAG, "Lib not found (linked statically or missing): $lib")
            }
        }

        sNativeLoaded = true
        Log.i(TAG, "Native libraries loaded: $loaded/${libs.size}")
    }
}