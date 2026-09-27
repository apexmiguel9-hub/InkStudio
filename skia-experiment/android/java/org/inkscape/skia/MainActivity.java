package org.inkscape.skia;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;

import java.util.ArrayList;
import java.util.List;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Skia experiment with the THREE tools ported from the ThorVG app
 * (prototype/) so the engine comparison stays fair:
 *   Selector (mover)  — tap selects (8 scale handles), next tap on the
 *                       selected rect = 4 rotate nodes, drag moves/resizes
 *   Rectangulo        — drag creates a persistent new rect (previous ones stay)
 *   Nodos             — corner nodes resize the rect
 * The toolbar strip below is a faithful copy of the prototype's Toolbox.
 */
public class MainActivity extends Activity {

    private static final int TOOL_SELECT = 0; // default tool
    private static final int TOOL_RECT = 1;
    private static final int TOOL_NODE = 2;   // Node tool (Option 1: edit rect corners)

    static {
        System.loadLibrary("skia_app");
    }

    private GLSurfaceView glView;

    public static native boolean nativeInit(int w, int h);
    public static native void nativeDraw();
    public static native void nativePresent();
    public static native void nativeTouch(int action, float x, float y); // 0 down 1 up 2 move
    public static native void nativeSetTool(int tool);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        // Keep the toolbar below the status bar / cutout (same as the
        // prototype: "el botón está muy arriba" lesson).
        root.setFitsSystemWindows(true);

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
                case MotionEvent.ACTION_UP:     action = 1; break;
                case MotionEvent.ACTION_CANCEL: action = 1; break;
                default:                        action = 2; break; // MOVE
            }
            nativeTouch(action, event.getX(), event.getY());
            glView.requestRender();
            return true;
        });

        // Tool strip goes on top (visual order = addView order).
        final Toolbox toolbox = new Toolbox(this, glView);
        toolbox.addTool(TOOL_SELECT, "\u2316  Selector");
        toolbox.addTool(TOOL_RECT, "\u25A1  Rect\u00E1ngulo");
        toolbox.addTool(TOOL_NODE, "\u25CF  Nodos");
        toolbox.select(TOOL_SELECT);
        root.addView(toolbox, new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        root.addView(glView, new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, 0, 1.0f));

        setContentView(root);
    }

    /** Reusable tool strip — faithful port of the prototype's Toolbox, with
     *  the selection tracked per-tool id (tag) instead of by button index. */
    private static class Toolbox extends LinearLayout {
        private final List<Button> buttons = new ArrayList<Button>();
        private final GLSurfaceView surface;
        private int current = -1;

        Toolbox(Context c, GLSurfaceView view) {
            super(c);
            surface = view;
            setOrientation(HORIZONTAL);
            setPadding(dp(8), dp(8), dp(8), dp(8));
            setBackgroundColor(0xFF1E1F26);
        }

        int dp(int v) {
            return (int) (v * getResources().getDisplayMetrics().density + 0.5f);
        }

        void addTool(final int id, String label) {
            Button b = new Button(getContext());
            b.setText(label);
            b.setAllCaps(false);
            b.setTag(id);
            b.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    select(id);
                    nativeSetTool(id);
                    surface.requestRender();
                }
            });
            LayoutParams lp = new LayoutParams(
                    ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
            lp.rightMargin = dp(8);
            buttons.add(b);
            addView(b, lp);
        }

        void select(int id) {
            current = id;
            for (Button b : buttons) {
                boolean sel = ((Integer) b.getTag()) == id;
                b.setSelected(sel);
                b.setBackgroundColor(sel ? 0xFF2E5FFF : 0xFF33333C);
                b.setTextColor(sel ? Color.WHITE : 0xFFB9BCC7);
            }
        }
    }
}