// ***************************************************************************
// Copyright (c) 2021 SAP SE or an SAP affiliate company. All rights reserved.
// ***************************************************************************
var exec = require('child_process').exec;

function doBuild() {
    console.log("Attempting to build binaries from source...");
    try {
        var config = exec("node-gyp configure", function (error, out) {
            if (error) {
                console.error("Error during node-gyp configure:", error, out);
                process.exit(1);
            }
            var build = exec("node-gyp build", function (error, out) {
                if (error) {
                    console.error("Error during node-gyp build:", error, out);
                    console.error("Please ensure a C++ toolchain is installed and configured correctly.");
                    process.exit(1);
                }
                console.log("Successfully built binaries!");
            });
        });
    } catch (err) {
        console.error("An unexpected error occurred during the build process.", err);
        process.exit(1);
    }
}

// The CI environment variable is a standard way to detect if the script
// is running in an automated environment like GitHub Actions.
if (process.env.CI) {
    // In a CI environment, we always want to build from scratch.
    console.log("CI environment detected. Forcing a rebuild.");
    doBuild();
} else {
    // When running locally, we first check if a binary already exists.
    // This allows you to use pre-compiled binaries without rebuilding every time.
    try {
        require("./lib/index");
        console.log("Pre-compiled binary found! Skipping build.");
    } catch (err) {
        // If the binary doesn't exist or is invalid, we fall back to building it.
        console.log("No pre-compiled binary found or an error occurred.");
        doBuild();
    }
}