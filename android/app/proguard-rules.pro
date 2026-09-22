# ProGuard rules for InkStudio

# Keep JNI native methods
-keepclasseswithmembernames class * {
    native <methods>;
}

# Keep InkscapeEngine and its native methods
-keep class org.inkscape.android.InkscapeEngine {
    *;
}

# Keep MainActivity
-keep class org.inkscape.android.MainActivity {
    *;
}

# Keep Application class
-keep class org.inkscape.android.InkStudioApplication {
    *;
}

# Keep JNI bridge
-keep class org.inkscape.android.** { *; }

# Keep Kotlin coroutines
-keep class kotlinx.coroutines.** { *; }

# Keep AndroidX
-keep class androidx.** { *; }

# Keep Material
-keep class com.google.android.material.** { *; }

# Don't obfuscate enums
-keepclassmembers enum * {
    public static **[] values();
    public static ** valueOf(java.lang.String);
}

# Keep annotations
-keepattributes *Annotation*
-keepattributes Signature
-keepattributes InnerClasses
-keepattributes EnclosingMethod