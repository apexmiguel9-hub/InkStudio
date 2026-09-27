package org.inkscape.skia;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.MotionEvent;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Minimal Skia experiment: GLSurfaceView rendering through native Skia
 * (Ganesh GL). Tap+drag draws a rectangle, same gesture as the ThorVG/Rive
 * prototypes.
 */
public class MainActivity extends Activity {

    static {
        System.loadLibrary("skia_app");
    }

    private GLSurfaceView glView;

    public native boolean nativeInit(int w, int h);
    public native void nativeDraw();
    public native void nativePresent();
    public native void nativeTouch(int action, float x, float y);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        glView = new GLSurfaceView(this);
        glView.setEGLContextClientVersion(2);
        glView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                nativeInit(width, height);
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                nativeDraw();
                nativePresent();
            }
        });
        glView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        glView.setOnTouchListener((view, event) -> {
            int action;
            switch (event.getActionMasked()) {
                case MotionEvent.ACTION_DOWN:   action = 0; break;
                case MotionEvent.ACTION_MOVE:   action = 2; break;
                case MotionEvent.ACTION_CANCEL: action = 3; break;
                default:                        action = 1; break;  // UP
            }
            nativeTouch(action, event.getX(), event.getY());
            glView.requestRender();
            return true;
        });

        setContentView(glView);
    }
}