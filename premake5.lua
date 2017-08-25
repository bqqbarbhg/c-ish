workspace "c-ish"
	configurations { "Debug", "Release" }
	platforms { "x86", "x64" }
	location "build"
	includedirs { "src" }
	flags { "C++11" }

	filter "action:vs*"
		defines { "_CRT_SECURE_NO_WARNINGS" }

	filter "configurations:Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		flags { "LinkTimeOptimization" }

	filter "platforms:x86"
		architecture "x86"

	filter "platforms:x64"
		architecture "x86_64"

project "base"
	kind "StaticLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	files { "src/base/**.h", "src/base/**.cpp" }

project "compiler"
	kind "StaticLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	files { "src/compiler/**.h", "src/compiler/**.cpp" }
	links { "base" }

project "test"
	kind "ConsoleApp"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	files { "src/test/**.h", "src/test/**.cpp" }
	links { "base", "compiler" }

