/*
 * Copyright (c) 2024 Florian "sp1rit" <sp1rit@disroot.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

package org.gtk.android;

import static java.lang.annotation.ElementType.CONSTRUCTOR;
import static java.lang.annotation.ElementType.METHOD;

import androidx.annotation.UiThread;

import java.lang.annotation.Target;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicReference;
import java.util.function.Supplier;

public final class GlibContext {
	@Target({METHOD, CONSTRUCTOR})
	public @interface GtkThread {}

	private GlibContext() {}

	public static native void runOnMain(Runnable runnable);

	/**
	 * FASE 6B: aplica el cambio de night-mode pendiente (marcado durante el
	 * bind inicial, cuando el hilo principal aún esperaba el latch de
	 * blockForMain). Consume el pendiente antes de emitir setting_changed +
	 * notify: no-op si no hay nada pendiente (no emite notify dos veces).
	 * Debe invocarse DESPUÉS de blockForMain() y vía runOnMain() para que
	 * corra en el hilo GTK con el hilo principal libre.
	 */
	public static native void commitPendingNightMode();

	@UiThread
	public static <T> T blockForMain(Supplier<T> runnable) {
		CountDownLatch barrier = new CountDownLatch(1);
		AtomicReference<T> retval = new AtomicReference<>();
		runOnMain(() -> {
			retval.set(runnable.get());
			barrier.countDown();
		});
		try {
			barrier.await();
		} catch (InterruptedException e) {
			// I don't think this can ever be thrown.
			throw new RuntimeException(e);
		}
		return retval.get();
	}

	@UiThread
	public static void blockForMain(Runnable runnable) {
		blockForMain(() -> {
			runnable.run();
			return null;
		});
	}
}
