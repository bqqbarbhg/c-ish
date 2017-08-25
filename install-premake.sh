#!/usr/bin/env bash
if [ ! -f premake_build/premake5 ]; then
	mkdir premake_build
	cd premake_build
	wget https://github.com/premake/premake-core/releases/download/v5.0.0-alpha12/premake-5.0.0-alpha12-src.zip -O premake.zip
	unzip -qq premake.zip
	cd premake-5.0.0-alpha12/build/gmake.unix
	make config=release
	cd ../../..
	mv premake-5.0.0-alpha12/bin/release/premake5 premake5
	cd ..
fi
