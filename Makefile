.PHONY: debug release clean run

debug:
	cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build cmake-build-debug

release:
	cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
	cmake --build cmake-build-release

run:
	./cmake-build-release/itch_parser

clean:
	rm -rf cmake-build-debug cmake-build-release
