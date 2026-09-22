package org.inkscape.android

import android.content.Context
import android.graphics.SurfaceTexture
import android.util.Log
import android.view.Surface
import android.view.TextureView
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

/**
 * Motor JNI para Inkscape en Android.
 * Inicializa GTK4/GDK backend Android y maneja el ciclo de vida del canvas.
 */
class InkscapeEngine(private val context: Context) {

    companion object {
        private const val TAG = "InkscapeEngine"
        // Códigos de retorno nativos
        const val INIT_OK = 0
        const val INIT_ERROR_NO_DISPLAY = -1
        const val INIT_ERROR_GTK_INIT = -2
        const val INIT_ERROR_SURFACE = -3
    }

    // Estado
    private var _isInitialized = false
    private var _surfaceWidth = 0
    private var _surfaceHeight = 0
    private var _surfaceDensity = 1.0f

    // Coroutine scope para callbacks async
    private val scope = CoroutineScope(Dispatchers.Default)

    init {
        // Verificar que las libs nativas están cargadas
        if (!InkStudioApplication.isNativeLoaded()) {
            Log.e(TAG, "Native libraries NOT loaded! Call InkStudioApplication.loadNativeLibraries() first")
        }
    }

    // ========== MÉTODOS NATIVOS (JNI) ==========
    external fun nativeInit(display: Long, width: Int, height: Int, density: Float): Int
    external fun nativeResize(width: Int, height: Int, density: Float): Int
    external fun nativeRender(): Int
    external fun nativeShutdown(): Int

    // Input events
    external fun nativeTouchDown(pointerId: Int, x: Float, y: Float, pressure: Float): Boolean
    external fun nativeTouchMove(pointerId: Int, x: Float, y: Float, pressure: Float): Boolean
    external fun nativeTouchUp(pointerId: Int, x: Float, y: Float): Boolean
    external fun nativeTouchCancel(pointerId: Int): Boolean
    external fun nativeScroll(dx: Float, dy: Float): Boolean
    external fun nativeScale(scaleFactor: Float, focusX: Float, focusY: Float): Boolean

    // Archivos
    external fun nativeOpenFile(path: String): Boolean
    external fun nativeSaveFile(path: String): Boolean
    external fun nativeNewDocument(): Boolean

    // ========== CICLO DE VIDA ==========

    /** Inicializa GTK4 + GDK Android backend con la Surface del TextureView */
    fun initialize(textureView: TextureView): Boolean {
        if (_isInitialized) {
            Log.w(TAG, "Already initialized")
            return true
        }

        val surfaceTexture = textureView.surfaceTexture
        if (surfaceTexture == null) {
            Log.e(TAG, "SurfaceTexture not available yet")
            return false
        }

        val surface = Surface(surfaceTexture)
        val display = 0L // TODO: obtener ANativeWindow_fromSurface via JNI si hace falta

        _surfaceWidth = textureView.width
        _surfaceHeight = textureView.height
        _surfaceDensity = context.resources.displayMetrics.density

        if (_surfaceWidth == 0 || _surfaceHeight == 0) {
            Log.e(TAG, "Invalid surface size: ${_surfaceWidth}x${_surfaceHeight}")
            return false
        }

        val result = nativeInit(display, _surfaceWidth, _surfaceHeight, _surfaceDensity)
        _isInitialized = result == INIT_OK

        if (_isInitialized) {
            Log.i(TAG, "InkscapeEngine initialized: ${_surfaceWidth}x${_surfaceHeight} @ ${_surfaceDensity}x")
            // Primer frame
            scope.launch { renderLoop() }
        } else {
            Log.e(TAG, "nativeInit failed with code: $result")
        }

        return _isInitialized
    }

    /** Redimensiona el canvas (rotación, multi-ventana) */
    fun resize(width: Int, height: Int, density: Float): Boolean {
        if (!_isInitialized) return false
        _surfaceWidth = width
        _surfaceHeight = height
        _surfaceDensity = density
        return nativeResize(width, height, density) == INIT_OK
    }

    /** Apaga el motor y libera recursos GTK */
    fun shutdown() {
        if (!_isInitialized) return
        nativeShutdown()
        _isInitialized = false
        scope.cancel()
        Log.i(TAG, "InkscapeEngine shutdown")
    }

    // ========== RENDER LOOP ==========

    private var _renderScheduled = false

    private fun renderLoop() {
        while (_isInitialized && !scope.isActive) {
            if (nativeRender() != INIT_OK) {
                Log.e(TAG, "nativeRender failed")
                break
            }
            // ~60fps target (16ms)
            try { Thread.sleep(16) } catch (e: InterruptedException) { break }
        }
    }

    /** Solicita un redibujado (desde UI thread) */
    fun requestRender() {
        if (!_isInitialized || _renderScheduled) return
        _renderScheduled = true
        scope.launch {
            nativeRender()
            _renderScheduled = false
        }
    }

    // ========== INPUT ==========

    fun onTouchDown(pointerId: Int, x: Float, y: Float, pressure: Float): Boolean {
        return _isInitialized && nativeTouchDown(pointerId, x, y, pressure)
    }

    fun onTouchMove(pointerId: Int, x: Float, y: Float, pressure: Float): Boolean {
        return _isInitialized && nativeTouchMove(pointerId, x, y, pressure)
    }

    fun onTouchUp(pointerId: Int, x: Float, y: Float): Boolean {
        return _isInitialized && nativeTouchUp(pointerId, x, y)
    }

    fun onTouchCancel(pointerId: Int): Boolean {
        return _isInitialized && nativeTouchCancel(pointerId)
    }

    fun onScroll(dx: Float, dy: Float): Boolean {
        return _isInitialized && nativeScroll(dx, dy)
    }

    fun onScale(scaleFactor: Float, focusX: Float, focusY: Float): Boolean {
        return _isInitialized && nativeScale(scaleFactor, focusX, focusY)
    }

    // ========== ARCHIVOS ==========

    fun openFile(path: String): Boolean = _isInitialized && nativeOpenFile(path)
    fun saveFile(path: String): Boolean = _isInitialized && nativeSaveFile(path)
    fun newDocument(): Boolean = _isInitialized && nativeNewDocument()

    // ========== GETTERS ==========

    val isInitialized: Boolean get() = _isInitialized
    val surfaceWidth: Int get() = _surfaceWidth
    val surfaceHeight: Int get() = _surfaceHeight
    val surfaceDensity: Float get() = _surfaceDensity
}