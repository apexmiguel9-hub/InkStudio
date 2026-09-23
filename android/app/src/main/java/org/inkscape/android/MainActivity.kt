package org.inkscape.android

import android.content.Intent
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import android.view.TextureView
import android.view.View
import android.widget.FrameLayout
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.WindowCompat
import androidx.lifecycle.lifecycleScope
import kotlin.math.abs
import kotlinx.coroutines.launch
import org.inkscape.android.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private lateinit var inkscapeEngine: InkscapeEngine
    private var _isEngineReady = false

    // Gesture detectors
    private lateinit var scaleDetector: ScaleGestureDetector
    private var _lastTouchX = 0f
    private var _lastTouchY = 0f
    private var _activePointers = mutableMapOf<Int, Pair<Float, Float>>()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Edge-to-edge
        WindowCompat.setDecorFitsSystemWindows(window, false)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        inkscapeEngine = InkscapeEngine(this)

        setupTextureView()
        setupGestures()
        handleIntent(intent)

        // Observar cuando la surface esté lista
        binding.inkTextureView.surfaceTextureListener = object : TextureView.SurfaceTextureListener {
            override fun onSurfaceTextureAvailable(surface: android.graphics.SurfaceTexture, width: Int, height: Int) {
                Log.d("MainActivity", "Surface available: ${width}x${height}")
                initializeEngine()
            }
            override fun onSurfaceTextureSizeChanged(surface: android.graphics.SurfaceTexture, width: Int, height: Int) {
                Log.d("MainActivity", "Surface resized: ${width}x${height}")
                if (_isEngineReady) {
                    inkscapeEngine.resize(width, height, resources.displayMetrics.density)
                }
            }
            override fun onSurfaceTextureDestroyed(surface: android.graphics.SurfaceTexture): Boolean {
                Log.d("MainActivity", "Surface destroyed")
                inkscapeEngine.shutdown()
                _isEngineReady = false
                return true
            }
            override fun onSurfaceTextureUpdated(surface: android.graphics.SurfaceTexture) {}
        }
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        handleIntent(intent)
    }

    private fun handleIntent(intent: Intent) {
        if (intent.action == Intent.ACTION_VIEW) {
            val uri = intent.data
            if (uri != null) {
                lifecycleScope.launch {
                    // Esperar a que el engine esté listo
                    while (!_isEngineReady) { kotlinx.coroutines.delay(100) }
                    val path = uri.toString()
                    if (path.startsWith("content://") || path.startsWith("file://")) {
                        inkscapeEngine.openFile(path)
                    }
                }
            }
        }
    }

    private fun initializeEngine() {
        if (_isEngineReady) return

        val success = inkscapeEngine.initialize(binding.inkTextureView)
        _isEngineReady = success

        runOnUiThread {
            if (success) {
                binding.loadingOverlay.visibility = View.GONE
                Toast.makeText(this, "Inkscape ready!", Toast.LENGTH_SHORT).show()
            } else {
                binding.loadingText.text = "Error initializing Inkscape engine"
                binding.loadingText.setTextColor(0xFFFF0000.toInt())
                Toast.makeText(this, "Failed to initialize Inkscape", Toast.LENGTH_LONG).show()
            }
        }
    }

    private fun setupTextureView() {
        binding.inkTextureView.setOpaque(false)
        binding.inkTextureView.setKeepScreenOn(true)
    }

    private fun setupGestures() {
        scaleDetector = ScaleGestureDetector(this, object : ScaleGestureDetector.SimpleOnScaleGestureListener() {
            override fun onScale(detector: ScaleGestureDetector): Boolean {
                return inkscapeEngine.onScale(detector.scaleFactor, detector.focusX, detector.focusY)
            }
        })

        binding.inkTextureView.setOnTouchListener { _, event ->
            scaleDetector.onTouchEvent(event)
            handleTouchEvent(event)
            true
        }
    }

    private fun handleTouchEvent(event: MotionEvent): Boolean {
        val action = event.actionMasked
        val pointerIndex = event.actionIndex
        val pointerId = event.getPointerId(pointerIndex)
        val x = event.getX(pointerIndex)
        val y = event.getY(pointerIndex)
        val pressure = event.getPressure(pointerIndex)

        return when (action) {
            MotionEvent.ACTION_DOWN,
            MotionEvent.ACTION_POINTER_DOWN -> {
                _activePointers[pointerId] = x to y
                inkscapeEngine.onTouchDown(pointerId, x, y, pressure)
            }
            MotionEvent.ACTION_MOVE -> {
                // Multi-touch: procesar todos los punteros activos
                for (i in 0 until event.pointerCount) {
                    val pid = event.getPointerId(i)
                    val px = event.getX(i)
                    val py = event.getY(i)
                    val p = event.getPressure(i)
                    val prev = _activePointers[pid] ?: (px to py)
                    val dx = px - prev.first
                    val dy = py - prev.second
                    if (abs(dx) > 1f || abs(dy) > 1f) {
                        inkscapeEngine.onTouchMove(pid, px, py, p)
                        _activePointers[pid] = px to py
                    }
                }
                true
            }
            MotionEvent.ACTION_UP,
            MotionEvent.ACTION_POINTER_UP -> {
                inkscapeEngine.onTouchUp(pointerId, x, y)
                _activePointers.remove(pointerId)
                true
            }
            MotionEvent.ACTION_CANCEL -> {
                inkscapeEngine.onTouchCancel(pointerId)
                _activePointers.remove(pointerId)
                true
            }
            else -> false
        }
    }

    override fun onDestroy() {
        inkscapeEngine.shutdown()
        super.onDestroy()
    }

    override fun onBackPressed() {
        // Si hay documento sin guardar, confirmar
        super.onBackPressed()
    }

    companion object {
        private const val TAG = "MainActivity"
    }
}