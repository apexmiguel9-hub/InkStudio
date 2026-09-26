package org.inkscape.alpha;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;

import java.util.ArrayList;
import java.util.List;

/**
 * InkAlpha — native canvas prototype (NDK + Rive low-level Vulkan renderer,
 * RIVE EXPERIMENT branch).
 *
 * Same two tools as the ThorVG version, both ported from Inkscape's ui/tools
 * (see prototype/cpp/tool/): the Select tool (move + rubberband + scale/
 * rotate handles) and the Rect tool.
 *
 * Engine note: the canvas is a plain SurfaceView whose android.view.Surface is
 * handed to native code (nativeSurface) — the native side creates a Vulkan
 * swapchain directly on the ANativeWindow (no GL, no intermediate layers).
 * All touch/tool state lives on the native render thread.
 */
public class MainActivity extends Activity {

    private static final int TOOL_SELECT = 0; // default tool
    private static final int TOOL_RECT = 1;

    private SurfaceView canvas;
    private Toolbox toolbox;

    // ---- native -----------------------------------------------------------
    static {
        System.loadLibrary("inkalpha");
    }

    private static native void nativeInit();
    private static native void nativeSurface(android.view.Surface surface); // null = destroyed
    private static native void nativeResize(int w, int h);
    private static native void nativeFrame();
    private static native void nativeTouch(float x, float y, int action); // 0 down 1 move 2 up
    private static native void nativeSetTool(int tool);

    // ---- activity ---------------------------------------------------------
    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);

        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        // Keep the toolbar below the status bar / cutout: the system insets
        // become root padding instead of letting the strip bleed under the
        // notch ("el botón está muy arriba" lesson 😄).
        root.setFitsSystemWindows(true);

        toolbox = new Toolbox(this);
        toolbox.addTool(TOOL_SELECT, "\u2316  Selector");
        toolbox.addTool(TOOL_RECT, "\u25A1  Rect\u00E1ngulo");
        toolbox.select(TOOL_SELECT);
        root.addView(toolbox, new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        canvas = new SurfaceView(this);
        canvas.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                nativeSurface(holder.getSurface());
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
                nativeResize(w, h);
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                nativeSurface(null);
            }
        });
        canvas.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent e) {
                int action;
                switch (e.getActionMasked()) {
                    case MotionEvent.ACTION_DOWN:
                        action = 0;
                        break;
                    case MotionEvent.ACTION_MOVE:
                        action = 1;
                        break;
                    default: // UP / CANCEL
                        action = 2;
                        break;
                }
                // Snapshot coords now (MotionEvent is recycled after onTouch);
                // the native side enqueues to the render thread (same-thread
                // rule as the old GL pipeline: tool+registry live there).
                nativeTouch(e.getX(), e.getY(), action);
                return true;
            }
        });
        root.addView(canvas, new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, 0, 1f));

        setContentView(root);
        nativeInit();
    }

    @Override
    protected void onPause() {
        super.onPause();
        // The framework destroys the surface; nativeSurface(null) fires via
        // the holder callback and tears down the Vulkan swapchain.
    }

    // ---- Toolbox (minimal reusable tool strip) ---------------------------
    private static class Toolbox extends LinearLayout {
        private final List<Button> buttons = new ArrayList<Button>();
        private int current = -1;

        Toolbox(Context c) {
            super(c);
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
            b.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    select(id);
                    nativeSetTool(id);
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
            for (int i = 0; i < buttons.size(); i++) {
                Button b = buttons.get(i);
                boolean sel = (i == current);
                b.setSelected(sel);
                b.setBackgroundColor(sel ? 0xFF2E5FFF : 0xFF33333C);
                b.setTextColor(sel ? Color.WHITE : 0xFFB9BCC7);
            }
        }
    }
}