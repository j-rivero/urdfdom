#!/bin/bash

# URDF-DOM Coverage Analysis Script
# This script builds the project with coverage enabled and generates an HTML report

set -e

echo "🔧 Building urdfdom with coverage enabled..."
colcon build --merge-install --cmake-args -DENABLE_COVERAGE=ON

echo "🧪 Running tests and generating coverage report..."
cd build/urdfdom
make test-coverage

echo "📊 Coverage Summary:"
echo "==================="
lcov --summary coverage_filtered.info

echo ""
echo "📁 Detailed Coverage by File:"
echo "============================="
lcov --list coverage_filtered.info

echo ""
echo "✅ Coverage analysis complete!"
echo "📄 HTML report available at: build/urdfdom/coverage_html/index.html"
echo "🌐 Open the report with: firefox build/urdfdom/coverage_html/index.html"