// ***************************************************************************
// Copyright (c) 2021 SAP SE or an SAP affiliate company. All rights reserved.
// ***************************************************************************
var db = null;

try {
    // prebuild-install or a local compile will place the binary here.
    db = require("../build/Release/sqlanywhere.node");
} catch (err) {
    // If the above fails, it's a genuine error (e.g., download failed and
    // local compile failed). We re-throw to provide a clear error message.
    console.error("Could not load the sqlanywhere driver.", err);
    console.error("This could be because a pre-compiled binary was not available for your platform and the fallback compilation failed.");
    console.error("Please ensure you have a C++ compiler and the necessary build tools installed.");
    throw new Error("Failed to load sqlanywhere native addon.");
}

module.exports = db;