// ***************************************************************************
// Copyright (c) 2021 SAP SE or an SAP affiliate company. All rights reserved.
// ***************************************************************************

// 'node-gyp-build' automatically checks the 'prebuilds/' folder (created by prebuildify)
// and falls back to 'build/Release/' (created by node-gyp rebuild).
module.exports = require('node-gyp-build')(__dirname)
