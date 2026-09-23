plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "org.inkscape.android"
    compileSdk = 34
    ndkVersion = "30.0.16248370"

    defaultConfig {
        applicationId = "org.inkscape.android"
        minSdk = 29
        targetSdk = 34
        versionCode = 1
        versionName = "1.0.0"

        // Native libs están en jniLibs/arm64-v8a/
        ndk {
            abiFilters.add("arm64-v8a")
        }

        // Habilitar C++17 para JNI bridge
        externalNativeBuild {
            cmake {
                cppFlags.add("-std=c++17")
                arguments.add("-DANDROID_STL=c++_static")
            }
        }
    }

    // NDK: SOLO via ndk.dir en local.properties (Gradle prohíbe ndkPath + ndk.dir juntos)
    // ndkPath = System.getenv("ANDROID_NDK_HOME") ?: "/usr/local/lib/android/sdk/ndk/26.1.10909125"

    signingConfigs {
        create("inkscape-release") {
            storeFile = file("keystore/inkscape-release.p12")
            storePassword = System.getenv("INKSIGN_PASS") ?: "inkscape2026"
            keyAlias = "inkscape"
            keyPassword = System.getenv("INKSIGN_PASS") ?: "inkscape2026"
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            isShrinkResources = false
            signingConfig = signingConfigs.getByName("inkscape-release")
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
        debug {
            isDebuggable = true
            isMinifyEnabled = false
        }
    }

    // Configuración de CMake para JNI bridge
    externalNativeBuild {
        cmake {
            path("src/main/cpp/CMakeLists.txt")
            // versión definida en CMakeLists.txt: cmake_minimum_required(VERSION 3.22.1)
        }
    }

    // Empaquetar .so nativas en el APK
    packaging {
        jniLibs {
            useLegacyPackaging = true
        }
    }

    // ViewBinding para ActivityMainBinding
    buildFeatures {
        viewBinding = true
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    kotlinOptions {
        jvmTarget = "17"
    }
}

dependencies {
    implementation("androidx.core:core-ktx:1.12.0")
    implementation("androidx.appcompat:appcompat:1.6.1")
    implementation("com.google.android.material:material:1.11.0")
    implementation("androidx.activity:activity-ktx:1.8.2")
    // Coroutines para async JNI calls
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-android:1.7.3")
    // Lifecycle para gestionar SurfaceView
    implementation("androidx.lifecycle:lifecycle-runtime-ktx:2.7.0")
}