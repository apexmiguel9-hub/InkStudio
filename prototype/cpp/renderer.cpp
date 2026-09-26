// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * renderer.cpp — Rive low-level Vulkan canvas for the alpha prototype.
 *
 * EXPERIMENT branch: this is the Rive port of the ThorVG renderer. The tool
 * logic, SPRect registry and touch dispatch are byte-for-byte the same as the
 * ThorVG version; ONLY the drawing engine differs:
 *
 *   ThorVG:  tvg::Shape + tvg::GlCanvas (EGL/GLES)
 *   Rive:    rive::gpu::RenderContext (Vulkan) + rive::RiveRenderer +
 *            rive::RenderPath/RenderPaint, presenting through an
 *            ANativeWindow swapchain (rive_vk_bootstrap).
 *
 * Frame flow (mirrors rive-runtime's own tests/common/testing_window_
 * android_vulkan.cpp, the in-repo reference for this exact integration):
 *   1. RenderContext::beginFrame(FrameDescriptor{...})
 *   2. swapchain->beginFrame()                    (acquire image)
 *   3. renderTarget->setTargetImageView(imageView, image, lastAccess)
 *   4. RiveRenderer r(renderContext); r.drawPath(...)
 *   5. renderContext->flush(FlushResources{target, cmdBuf, frameNumbers})
 *   6. swapchain->endFrame(targetLastAccess())    (present)
 *
 * Touch coords from Java (top-left origin, y-down, 1:1 with the doc in the
 * alpha) are synthesized into the tool's CanvasEvent interface (ButtonPress =
 * touch down, Motion with BUTTON1_MASK = drag, ButtonRelease = finger up),
 * the same normalized interface the Inkscape tool exposes.
 *
 * Dispatch (down/move/up) mirrors Inkscape's canvas grab precedence:
 *   1. a visible SelTrans handle under the finger (select tool) — the handle
 *      drag is driven straight by the engine, it never enters the tool;
 *   2. an item under the finger -> tool->item_handler() (if it consumes);
 *   3. otherwise               -> tool->root_handler().
 *
 * The alpha "document" is the live SPRect registry: every frame the renderer
 * redraws all registered rects applying SPRect::xform (identity or the
 * select tool's compose), plus the selection overlay (per-item transformed
 * box cue in #2A2D35 + scale/rotate handles) and the rubberband.
 */
#include "renderer.h"

#include <android/log.h>
#include <cmath>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

#include "rive/math/mat2d.hpp"
#include "rive/refcnt.hpp"
#include "rive/renderer.hpp" // Renderer base, RenderPath, RenderPaint, Factory
#include "rive/span.hpp"
#include "rive/renderer/rive_renderer.hpp"
#include "rive/renderer/render_context.hpp"
#include "rive/renderer/vulkan/render_context_vulkan_impl.hpp"
#include "rive/renderer/vulkan/render_target_vulkan.hpp"
#include "rive_vk_bootstrap/vulkan_device.hpp"
#include "rive_vk_bootstrap/vulkan_instance.hpp"
#include "rive_vk_bootstrap/vulkan_swapchain.hpp"

// rive_types.hpp defines `#define RELEASE 1` as a build-mode marker. The
// engine-agnostic canvas_events.h (byte-identical to the ThorVG port) has
// `RELEASE = 64` in its EventMask enum, so it must never see that macro.
// rive only references RELEASE in the `#ifndef RELEASE` self-guard at its own
// definition point (no later header re-checks it), so undefining here is safe
// for the rest of this translation unit.
#undef RELEASE

#include "canvas_events.h"
#include "ink_compat.h"     // GDK_BUTTON1_MASK
#include "sp_rect.h"        // sprect_registry(), SPRect
#include "desktop.h"        // SPDesktop (shared), SPDocument
#include "selection.h"      // Selection::items() for the cue overlay
#include "rubberband.h"     // Rubberband::get() overlay
#include "seltrans.h"       // active_seltrans(), handle overlay engine
#include "rect_tool_port.h"
#include "select_tool_port.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "inkalpha", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "inkalpha", __VA_ARGS__)

namespace gpu = rive::gpu;

// ---------------------------------------------------------------------------
// Path building helpers (the alpha only creates plain rects today; the
// rounded/circle paths are kept so the scene is geometry-equivalent to the
// ThorVG version, which also supported rx/ry and circular handles).
// ---------------------------------------------------------------------------

static void addRoundedRect(rive::RenderPath *p,
                           float x,
                           float y,
                           float w,
                           float h,
                           float rx,
                           float ry)
{
    const float k = 0.5522847498307936f; // circle approx constant
    rx = std::max(0.0f, std::min(rx, w * 0.5f));
    ry = std::max(0.0f, std::min(ry, h * 0.5f));
    p->moveTo(x + rx, y);
    p->lineTo(x + w - rx, y);
    p->cubicTo(x + w - rx + rx * k, y, x + w, y + ry - ry * k, x + w, y + ry);
    p->lineTo(x + w, y + h - ry);
    p->cubicTo(x + w, y + h - ry + ry * k, x + w - rx + rx * k, y + h,
               x + w - rx, y + h);
    p->lineTo(x + rx, y + h);
    p->cubicTo(x + rx - rx * k, y + h, x, y + h - ry + ry * k, x, y + h - ry);
    p->lineTo(x, y + ry);
    p->cubicTo(x, y + ry - ry * k, x + rx - rx * k, y, x + rx, y);
    p->close();
}

static void addCircle(rive::RenderPath *p, float cx, float cy, float r)
{
    const float k = 0.5522847498307936f;
    p->moveTo(cx - r, cy);
    p->cubicTo(cx - r, cy - k * r, cx - k * r, cy - r, cx, cy - r);
    p->cubicTo(cx + k * r, cy - r, cx + r, cy - k * r, cx + r, cy);
    p->cubicTo(cx + r, cy + k * r, cx + k * r, cy + r, cx, cy + r);
    p->cubicTo(cx - k * r, cy + r, cx - r, cy + k * r, cx - r, cy);
    p->close();
}

// Doc Xform (x'=a·x+c·y+e / y'=b·x+d·y+f) -> rive::Mat2D.
// rive::Mat2D(x1,y1,x2,y2,tx,ty) has xx()=x1, xy()=y1, yx()=x2, yy()=y2,
// tx()=tx, ty()=ty — i.e. the same row-vector convention as Geom::Xform:
// x' = xx·x + yx·y + tx. So Mat2D(a, b, c, d, e, f) is EXACTLY the doc
// transform (same mapping we used for tvg::Matrix's e11..e23).
static rive::Mat2D toRive(Geom::Xform const &xf)
{
    return rive::Mat2D((float)xf.a, (float)xf.b, (float)xf.c, (float)xf.d,
                       (float)xf.e, (float)xf.f);
}

static bool isIdentity(Geom::Xform const &xf)
{
    return xf.a == 1.0 && xf.b == 0.0 && xf.c == 0.0 && xf.d == 1.0 &&
           xf.e == 0.0 && xf.f == 0.0;
}

// ---------------------------------------------------------------------------
// Renderer
// ---------------------------------------------------------------------------

Renderer::Renderer() = default;

Renderer::~Renderer()
{
    {
        std::lock_guard<std::mutex> lk(mutex_);
        exit_ = true;
    }
    cv_.notify_all();
    if (thread_.joinable()) {
        thread_.join();
    }
    // destroyCanvas() runs on the render thread; the thread is joined above.
    delete tool_;
    delete desktop_; // document is process-wide; desktop is not
}

void Renderer::init()
{
    std::lock_guard<std::mutex> lk(mutex_);
    if (thread_.joinable()) {
        return;
    }
    thread_ = std::thread([this] { threadMain(); });
}

void Renderer::push(Task t)
{
    {
        std::lock_guard<std::mutex> lk(mutex_);
        queue_.push_back(std::move(t));
    }
    cv_.notify_all();
}

void Renderer::setWindow(ANativeWindow *w)
{
    Task t;
    t.type = TaskType::SetWindow;
    t.w = w; // take ownership of this reference (may be null)
    push(std::move(t));
}

void Renderer::resize(int /*w*/, int /*h*/)
{
    Task t;
    t.type = TaskType::Resize;
    push(std::move(t));
}

void Renderer::requestFrame()
{
    Task t;
    t.type = TaskType::Frame;
    push(std::move(t));
}

void Renderer::setTool(int tool)
{
    Task t;
    t.type = TaskType::SetTool;
    t.i = tool;
    push(std::move(t));
}

void Renderer::touch(float x, float y, int action)
{
    Task t;
    t.type = TaskType::Touch;
    t.x = x;
    t.y = y;
    t.i = action;
    push(std::move(t));
}

void Renderer::threadMain()
{
    for (;;) {
        std::vector<Task> tasks;
        {
            std::unique_lock<std::mutex> lk(mutex_);
            cv_.wait(lk, [this] { return !queue_.empty() || exit_; });
            while (!queue_.empty()) {
                tasks.push_back(std::move(queue_.front()));
                queue_.pop_front();
            }
            if (exit_ && tasks.empty()) {
                return;
            }
            if (exit_) {
                // drain remaining tasks before the final frame
            }
        }

        for (Task const &t : tasks) {
            switch (t.type) {
                case TaskType::SetWindow: {
                    if (window_ != nullptr) {
                        ANativeWindow_release(window_);
                    }
                    window_ = t.w;
                    if (window_ == nullptr) {
                        destroyCanvas();
                    } else {
                        ensureCanvas();
                    }
                    dirty_ = true;
                    break;
                }
                case TaskType::Resize:
                    // The swapchain size is authoritative; a stale resize just
                    // triggers a redraw. A real size change destroys/rebuilds
                    // the surface on Android (surfaceDestroyed/Created).
                    dirty_ = true;
                    break;
                case TaskType::Touch:
                    touchImpl(t.x, t.y, t.i); // NB: keep below as member
                    dirty_ = true;
                    break;
                case TaskType::SetTool:
                    setToolImpl(t.i);
                    dirty_ = true;
                    break;
                case TaskType::Frame:
                    dirty_ = true;
                    break;
                case TaskType::Exit:
                    return;
            }
        }

        if (dirty_) {
            dirty_ = false;
            renderFrame();
        }
    }
}

// (implemented below; render thread only)
void Renderer::recreateTool()
{
    // The tools are cheap; switching tools destroys the old instance (which
    // clears active_seltrans()) and constructs the new one on the SAME
    // desktop — the selection/document state persists across switches.
    delete tool_;
    tool_ = nullptr;
    if (!desktop_) {
        desktop_ = new SPDesktop();
    }
    if (toolId_ == 1) { // TOOL_RECT
        tool_ = new Inkscape::UI::Tools::RectTool(desktop_);
    } else {            // 0 = TOOL_SELECT (default)
        tool_ = new Inkscape::UI::Tools::SelectTool(desktop_);
    }
    touchDown_ = false;
    handleDrag_ = false;
}

void Renderer::ensureTool()
{
    if (!tool_) {
        recreateTool();
    }
}

void Renderer::setToolImpl(int t)
{
    if (t == toolId_ && tool_) {
        return; // already on this tool
    }
    toolId_ = t;
    recreateTool();
}

void Renderer::touchImpl(float x, float y, int action)
{
    ensureTool();

    Geom::Point const p(x, y);

    if (action == 0) { // DOWN
        if (touchDown_) {
            // Safety: complete an unterminated gesture first.
            Inkscape::UI::ButtonReleaseEvent up;
            up.pos = p;
            up.button = 1;
            up.modifiers = 0;
            SPItem *item = desktop_->getItemAtPoint(p, false);
            dispatchEvent(item, up);
        }
        touchDown_ = true;
        desktop_->setPoint(p);

        // 1. SelTrans handle (select tool only; active_seltrans() is null
        //    while the rect tool is active). The handle grab never enters the
        //    tool: like Inkscape's knots, it sits above the drawing.
        if (Inkscape::SelTrans *st = Inkscape::active_seltrans();
            st && st->tryGrabHandle(p)) {
            handleDrag_ = true;
            return;
        }

        // 2./3. item -> root
        Inkscape::UI::ButtonPressEvent down;
        down.pos = p;
        down.orig_pos = p;
        down.button = 1;
        down.num_press = 1;
        down.modifiers = 0;
        SPItem *item = desktop_->getItemAtPoint(p, false);
        dispatchEvent(item, down);
    } else if (action == 1 && touchDown_) { // MOVE
        desktop_->setPoint(p);
        if (handleDrag_) {
            if (Inkscape::SelTrans *st = Inkscape::active_seltrans(); st) {
                st->moveHandle(p);
            }
            return;
        }
        Inkscape::UI::MotionEvent move;
        move.pos = p;
        move.orig_pos = p;
        move.modifiers = GDK_BUTTON1_MASK; // drag while the finger is down
        SPItem *item = desktop_->getItemAtPoint(p, false);
        dispatchEvent(item, move);
    } else if (action == 2) { // UP / CANCEL
        if (!touchDown_ && !handleDrag_) {
            return;
        }
        touchDown_ = false;
        if (handleDrag_) {
            handleDrag_ = false;
            if (Inkscape::SelTrans *st = Inkscape::active_seltrans(); st) {
                st->releaseHandle();
            }
            return;
        }
        Inkscape::UI::ButtonReleaseEvent up;
        up.pos = p;
        up.button = 1;
        up.modifiers = 0;
        SPItem *item = desktop_->getItemAtPoint(p, false);
        dispatchEvent(item, up);
    }
}

void Renderer::dispatchEvent(SPItem *item, Inkscape::UI::CanvasEvent const &ev)
{
    if (item && tool_->item_handler(item, ev)) {
        return;
    }
    tool_->root_handler(ev);
}

// ---------------------------------------------------------------------------
// Vulkan / Rive setup — mirror of rive-runtime tests/common/
// testing_window_android_vulkan.cpp (the in-repo Android+Vulkan reference).
// ---------------------------------------------------------------------------

void Renderer::ensureCanvas()
{
    if (vkReady_ || window_ == nullptr) {
        return;
    }
    using namespace rive_vkb;

    LOGI("Rive Vulkan: creating instance/device/swapchain");

    std::vector<const char *> extensions = {VK_KHR_SURFACE_EXTENSION_NAME,
                                            VK_KHR_ANDROID_SURFACE_EXTENSION_NAME};
    vkInstance_ = VulkanInstance::Create(VulkanInstance::Options{
        .appName = "InkAlpha (Rive experiment)",
        .engineName = "Rive Renderer",
        .idealAPIVersion = VK_API_VERSION_1_3,
        .minimumSupportedInstanceVersion = VK_API_VERSION_1_1,
        .requiredExtensions =
            rive::make_span(extensions.data(), extensions.size()),
        .optionalExtensions = {},
#ifndef NDEBUG
        .desiredValidationType = VulkanValidationType::core,
        .wantDebugCallbacks = true,
#endif
    });
    if (!vkInstance_) {
        LOGE("ensureCanvas: VulkanInstance::Create failed");
        return;
    }

    pfnDestroySurface_ =
        vkInstance_->loadInstanceFunc<PFN_vkDestroySurfaceKHR>(
            "vkDestroySurfaceKHR");

    VkAndroidSurfaceCreateInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        .window = window_,
    };
    auto makeSurface = vkInstance_->loadInstanceFunc<PFN_vkCreateAndroidSurfaceKHR>(
        "vkCreateAndroidSurfaceKHR");
    if (makeSurface == nullptr ||
        makeSurface(vkInstance_->vkInstance(), &info, nullptr,
                    &windowSurface_) != VK_SUCCESS) {
        LOGE("ensureCanvas: vkCreateAndroidSurfaceKHR failed");
        vkInstance_.reset();
        return;
    }

    vkDevice_ = VulkanDevice::Create(
        *vkInstance_,
        VulkanDevice::Options{
            .presentationSurfaceForDeviceSelection = windowSurface_,
        });
    if (!vkDevice_) {
        LOGE("ensureCanvas: VulkanDevice::Create failed");
        pfnDestroySurface_(vkInstance_->vkInstance(), windowSurface_, nullptr);
        windowSurface_ = VK_NULL_HANDLE;
        vkInstance_.reset();
        return;
    }

    renderContext_ = gpu::RenderContextVulkanImpl::MakeContext(
        vkInstance_->vkInstance(), vkDevice_->vkPhysicalDevice(),
        vkDevice_->vkDevice(), vkDevice_->vulkanFeatures(),
        vkInstance_->getVkGetInstanceProcAddrPtr(), {});
    if (!renderContext_) {
        LOGE("ensureCanvas: MakeContext failed");
        vkDevice_.reset();
        return;
    }

    VkSurfaceCapabilitiesKHR caps;
    if (vkDevice_->getSurfaceCapabilities(windowSurface_, &caps) != VK_SUCCESS) {
        LOGE("ensureCanvas: getSurfaceCapabilities failed");
        renderContext_.reset();
        vkDevice_.reset();
        return;
    }

    auto swapOpts = VulkanSwapchain::Options{
        .formatPreferences =
            {
                {
                    .format = VK_FORMAT_R8G8B8A8_UNORM,
                    .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
                },
                {
                    .format = VK_FORMAT_B8G8R8A8_UNORM,
                    .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
                },
                {
                    .format = VK_FORMAT_R8G8B8A8_SRGB,
                    .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
                },
            },
        .presentModePreferences =
            {
                VK_PRESENT_MODE_IMMEDIATE_KHR, // no vsync: draw-on-demand app
                VK_PRESENT_MODE_MAILBOX_KHR,
                VK_PRESENT_MODE_FIFO_KHR,
            },
        .imageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                           VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    };
    if ((caps.supportedUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) != 0) {
        swapOpts.imageUsageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }

    swapchain_ = VulkanSwapchain::Create(*vkInstance_, *vkDevice_,
                                         ref_rcp(renderContext_->static_impl_cast<
                                                 gpu::RenderContextVulkanImpl>()
                                                     ->vulkanContext()),
                                         windowSurface_, swapOpts);
    if (!swapchain_) {
        LOGE("ensureCanvas: VulkanSwapchain::Create failed");
        renderContext_.reset();
        vkDevice_.reset();
        return;
    }

    W_ = swapchain_->width();
    H_ = swapchain_->height();

    renderTarget_ = renderContext_->static_impl_cast<
                        gpu::RenderContextVulkanImpl>()
                        ->makeRenderTarget(W_, H_, swapchain_->imageFormat(),
                                           swapchain_->imageUsageFlags());
    vkReady_ = true;
    LOGI("Rive Vulkan ready: %ux%u (%s)", W_, H_,
         vkDevice_->name().c_str());
}

void Renderer::destroyCanvas()
{
    if (!vkReady_ && !vkDevice_ && !vkInstance_) {
        return;
    }
    LOGI("Rive Vulkan: tearing down");
    if (vkDevice_) {
        vkDevice_->waitUntilIdle();
    }
    swapchain_.reset();
    renderTarget_.reset();
    renderContext_.reset();
    vkDevice_.reset();
    if (windowSurface_ != VK_NULL_HANDLE && vkInstance_) {
        pfnDestroySurface_(vkInstance_->vkInstance(), windowSurface_, nullptr);
        windowSurface_ = VK_NULL_HANDLE;
    }
    vkInstance_.reset();
    vkReady_ = false;
}

// ---------------------------------------------------------------------------
// Frame — the scene is geometrically equivalent to the ThorVG version; the
// drawing API is Rive's low-level Renderer.
// ---------------------------------------------------------------------------

void Renderer::renderFrame()
{
    if (!vkReady_ || !renderContext_ || !swapchain_ || !renderTarget_) {
        return;
    }

    ensureTool(); // first frame may arrive before any touch

    // 1. begin the rive frame.
    renderContext_->beginFrame(gpu::RenderContext::FrameDescriptor{
        .renderTargetWidth = W_,
        .renderTargetHeight = H_,
        .loadAction = gpu::LoadAction::clear,
        .clearColor = 0xFFEFEFF4, // same clear as the ThorVG frame paints fully
        .msaaSampleCount = 0,     // let the renderer pick its interlock mode
        .disableRasterOrdering = false,
        .ditherMode = gpu::DitherMode::interleavedGradientNoise,
        .triangulationThresholds = {},
    });

    // 2/3. acquire the swapchain image and point the render target at it.
    if (!swapchain_->isFrameStarted()) {
        if (swapchain_->beginFrame() != VK_SUCCESS) {
            LOGE("renderFrame: swapchain->beginFrame failed; tearing down");
            destroyCanvas();
            return;
        }
        renderTarget_->setTargetImageView(swapchain_->currentVkImageView(),
                                          swapchain_->currentVkImage(),
                                          swapchain_->currentLastAccess());
    }

    rive::RiveRenderer renderer(renderContext_.get());
    rive::Factory *f = renderContext_.get();

    // ---- background (opaque, covers the viewport — same as ThorVG) --------
    {
        auto path = f->makeEmptyRenderPath();
        path->addRect(0.0f, 0.0f, (float)W_, (float)H_);
        auto paint = f->makeRenderPaint();
        paint->style(rive::RenderPaintStyle::fill);
        paint->color(0xFFEFEFF4); // 0.949f grey, ancient clear colour
        renderer.drawPath(path.get(), paint.get());
    }

    // ---- document: all registered rects with their xform -------------------
    for (SPRect *r : sprect_registry()) {
        float x = (float)r->x.computed;
        float y = (float)r->y.computed;
        float w = (float)r->width.computed;
        float h = (float)r->height.computed;
        if (w == 0.0f || h == 0.0f) {
            continue;
        }

        auto path = f->makeEmptyRenderPath();
        if (r->rx.computed == 0.0f && r->ry.computed == 0.0f) {
            path->addRect(x, y, w, h);
        } else {
            addRoundedRect(path.get(), x, y, w, h, (float)r->rx.computed,
                           (float)r->ry.computed);
        }

        auto fill = f->makeRenderPaint();
        fill->style(rive::RenderPaintStyle::fill);
        fill->color(0xF03366FF); // #3366FF, alpha 240

        auto stroke = f->makeRenderPaint();
        stroke->style(rive::RenderPaintStyle::stroke);
        stroke->thickness(1.5f);
        stroke->color(0xFF121420); // #121420
        stroke->join(rive::StrokeJoin::miter);
        stroke->cap(rive::StrokeCap::butt);

        renderer.save();
        if (!isIdentity(r->xform)) {
            renderer.transform(toRive(r->xform));
        }
        renderer.drawPath(path.get(), fill.get());
        renderer.drawPath(path.get(), stroke.get()); // fill then stroke, like ThorVG
        renderer.restore();
    }

    // ---- selection overlay (only while the select tool is active) ----------
    if (Inkscape::SelTrans *st = Inkscape::active_seltrans(); st && !st->isEmpty()) {
        // per-item cue: the *transformed* box outline (rotates with the fill,
        // Inkscape's "_bbox->corner(i) * affine" box) — #2A2D35
        for (SPItem *item : desktop_->getSelection()->items()) {
            auto *r = dynamic_cast<SPRect *>(item);
            if (!r || r->isEmpty()) {
                continue;
            }
            auto c = r->docCorners();
            auto path = f->makeEmptyRenderPath();
            path->moveTo((float)c[0].x, (float)c[0].y);
            path->lineTo((float)c[1].x, (float)c[1].y);
            path->lineTo((float)c[2].x, (float)c[2].y);
            path->lineTo((float)c[3].x, (float)c[3].y);
            path->close();
            auto paint = f->makeRenderPaint();
            paint->style(rive::RenderPaintStyle::stroke);
            paint->thickness(1.0f);
            paint->color(0xFF2A2D35);
            renderer.drawPath(path.get(), paint.get());
        }

        // scale/rotate handles (SCALE: 8 white 10x10 squares; ROTATE: 4 white
        // r=7 circles at the offset corners + center crosshair)
        if (st->visible()) {
            for (auto const &hp : st->handlePositions()) {
                auto path = f->makeEmptyRenderPath();
                if (st->state() == Inkscape::SelTrans::STATE_SCALE) {
                    path->addRect(hp.pos.x - 5.0f, hp.pos.y - 5.0f, 10.0f, 10.0f);
                } else if (hp.id == Inkscape::SelTrans::Handle::CENTER) {
                    // center crosshair (dark, thin): rotation pivot
                    auto b1 = f->makeEmptyRenderPath();
                    b1->addRect(hp.pos.x - 8.0f, hp.pos.y - 0.75f, 16.0f, 1.5f);
                    auto b2 = f->makeEmptyRenderPath();
                    b2->addRect(hp.pos.x - 0.75f, hp.pos.y - 8.0f, 1.5f, 16.0f);
                    auto paint = f->makeRenderPaint();
                    paint->style(rive::RenderPaintStyle::fill);
                    paint->color(0xFF101014);
                    renderer.drawPath(b1.get(), paint.get());
                    renderer.drawPath(b2.get(), paint.get());
                    continue;
                } else {
                    addCircle(path.get(), hp.pos.x, hp.pos.y, 7.0f);
                }

                auto fill = f->makeRenderPaint();
                fill->style(rive::RenderPaintStyle::fill);
                fill->color(0xFFFFFFFF);
                auto stroke = f->makeRenderPaint();
                stroke->style(rive::RenderPaintStyle::stroke);
                stroke->thickness(1.0f);
                stroke->color(0xFF101014);
                renderer.drawPath(path.get(), fill.get());
                renderer.drawPath(path.get(), stroke.get());
            }
        }
    }

    // ---- rubberband overlay (select tool drag on empty area) ---------------
    if (Inkscape::Rubberband *rb = Inkscape::Rubberband::get(desktop_);
        rb->isStarted() && rb->isMoved()) {
        if (auto b = rb->getRectangle()) {
            auto path = f->makeEmptyRenderPath();
            path->addRect((float)b->min().x, (float)b->min().y,
                          (float)b->width(), (float)b->height());
            auto fill = f->makeRenderPaint();
            fill->style(rive::RenderPaintStyle::fill);
            fill->color(0x3C3366FF); // #3366FF, alpha 60
            auto stroke = f->makeRenderPaint();
            stroke->style(rive::RenderPaintStyle::stroke);
            stroke->thickness(1.0f);
            stroke->color(0xFF2E5FFF);
            renderer.drawPath(path.get(), fill.get());
            renderer.drawPath(path.get(), stroke.get());
        }
    }

    // 5. flush + 6. present.
    renderContext_->flush({
        .renderTarget = renderTarget_.get(),
        .externalCommandBuffer = swapchain_->currentCommandBuffer(),
        .currentFrameNumber = swapchain_->currentFrameNumber(),
        .safeFrameNumber = swapchain_->safeFrameNumber(),
    });
    VkResult vr = swapchain_->endFrame(renderTarget_->targetLastAccess());
    if (vr != VK_SUCCESS) {
        // VK_ERROR_OUT_OF_DATE (surface changed) or DEVICE_LOST: drop the
        // canvas; the next surfaceCreated() rebuilds it.
        LOGE("renderFrame: swapchain->endFrame failed (0x%x); tearing down", vr);
        destroyCanvas();
    }
}