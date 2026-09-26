// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * renderer.h — Vulkan native canvas for the Rive experiment (alpha).
 *
 * Same contract as the ThorVG renderer (same JNI surface, same tool/registry
 * state), but the engine is Rive's low-level renderer:
 *   rive::gpu::RenderContext (Vulkan impl) + rive::RiveRenderer + RenderPath,
 * presenting straight to an ANativeWindow swapchain (no GL, no intermediate
 * layers).
 *
 * Threading: a dedicated render thread owns the tool + SPRect registry +
 * Vulkan state (single owner, mirrors the old GL thread rule). JNI calls just
 * enqueue tasks. The Vulkan surface arrives from Java as an ANativeWindow
 * (view -> surface -> nativeSurface()).
 *
 * Touch dispatch (Renderer::touch) mirrors Inkscape's canvas grab order:
 *   1. a visible SelTrans handle under the finger  -> seltrans handle drag
 *   2. an item under the finger                   -> tool->item_handler()
 *   3. otherwise                                  -> tool->root_handler()
 */
#ifndef INKALPHA_RENDERER_H
#define INKALPHA_RENDERER_H

#include <condition_variable>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>

#include <android/native_window.h>
#include <vulkan/vulkan.h>

#include "rive/refcnt.hpp"

namespace rive {
namespace gpu {
class RenderContext;
class RenderTargetVulkanImpl;
} // namespace gpu
} // namespace rive
namespace rive_vkb {
class VulkanInstance;
class VulkanDevice;
class VulkanSwapchain;
} // namespace rive_vkb

class SPDesktop;
class SPItem;
namespace Inkscape::UI {
struct CanvasEvent;
}
namespace Inkscape::UI::Tools {
class ToolBase;
}

class Renderer {
public:
    Renderer();
    ~Renderer();

    void init();                    // spawn the render thread (called once)
    void setWindow(ANativeWindow *w); // surface created (non-null) / destroyed (null)
    void resize(int w, int h);      // informational; swapchain size is authoritative
    void requestFrame();            // redraw on demand (JNI nativeFrame surface)
    void touch(float x, float y, int action); // 0=down 1=move 2=up/cancel
    void setTool(int tool);         // 0 = Select (default), 1 = Rect

private:
    enum class TaskType {
        SetWindow,
        Resize,
        Touch,
        SetTool,
        Frame,
        Exit,
    };
    struct Task {
        TaskType type;
        float x = 0.0f;
        float y = 0.0f;
        int i = 0;
        ANativeWindow *w = nullptr;
    };

    void push(Task t);
    void threadMain();
    void ensureCanvas();
    void destroyCanvas();
    void renderFrame();

    void ensureTool();
    void recreateTool();
    void touchImpl(float x, float y, int action);
    void setToolImpl(int tool);
    // item-under-finger first, root fallback (Inkscape grab precedence).
    void dispatchEvent(SPItem *item, Inkscape::UI::CanvasEvent const &ev);

    // ---- engine state (render thread only) ----
    ANativeWindow *window_ = nullptr; // borrowed ref from Java (owned here)
    uint32_t W_ = 0;
    uint32_t H_ = 0;
    bool vkReady_ = false;

    std::unique_ptr<rive_vkb::VulkanInstance> vkInstance_;
    std::unique_ptr<rive_vkb::VulkanDevice> vkDevice_;
    std::unique_ptr<rive_vkb::VulkanSwapchain> swapchain_;
    std::unique_ptr<rive::gpu::RenderContext> renderContext_;
    rive::rcp<rive::gpu::RenderTargetVulkanImpl> renderTarget_;
    VkSurfaceKHR windowSurface_ = VK_NULL_HANDLE;
    PFN_vkDestroySurfaceKHR pfnDestroySurface_ = nullptr;

    // ---- tool state (same as the ThorVG renderer; touched on render thread) -
    bool touchDown_ = false;
    bool handleDrag_ = false; // a SelTrans handle is being dragged
    int toolId_ = 0;          // TOOL_SELECT by default
    SPDesktop *desktop_ = nullptr;
    Inkscape::UI::Tools::ToolBase *tool_ = nullptr;

    // ---- threading ----
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::deque<Task> queue_;
    bool exit_ = false;
    bool dirty_ = false;
};

#endif // INKALPHA_RENDERER_H