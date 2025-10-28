MAX_PACKAGE_PATH="/Users/risse/Documents/Max 9/Packages"
PACKAGE_NAME="NlBend"

rm -rf "./$PACKAGE_NAME/build/" || true
mkdir -p "./$PACKAGE_NAME/build"
cd "./$PACKAGE_NAME/build"
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
cmake --build . --config "Release"

mkdir -p "$MAX_PACKAGE_PATH/$PACKAGE_NAME/externals"

rm -r "$MAX_PACKAGE_PATH/$PACKAGE_NAME/externals"
cd ../../
cp -r "$PACKAGE_NAME/externals/" "$MAX_PACKAGE_PATH/$PACKAGE_NAME/externals"

# Optionally expose compile_commands.json for IntelliSense
if [ -f "./$PACKAGE_NAME/build/compile_commands.json" ]; then
	ln -sf "$(pwd)/$PACKAGE_NAME/build/compile_commands.json" "$(git rev-parse --show-toplevel 2>/dev/null || echo $(pwd)/../../..)/compile_commands.json" >/dev/null 2>&1 || true
fi
