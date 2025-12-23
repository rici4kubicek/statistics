#!/usr/bin/env python3
"""
Inject version selector into all built documentation versions.

This script copies the version selector template and static files from the main
branch into all other version builds, ensuring the version selector works across
all versions even if they didn't originally have it.
"""

import os
import shutil
import glob
from pathlib import Path

def inject_version_selector():
    """Inject version selector files into all version directories."""

    # Get paths
    script_dir = Path(__file__).parent
    source_dir = script_dir / 'source'
    build_dir = script_dir / 'build' / 'html'

    # Source files (from current/main version)
    template_file = source_dir / '_templates' / 'layout.html'
    css_file = source_dir / '_static' / 'version-selector.css'
    js_file = source_dir / '_static' / 'version-selector.js'

    # Check if source files exist
    if not all([template_file.exists(), css_file.exists(), js_file.exists()]):
        print("⚠ Warning: Version selector source files not found")
        print(f"  Template: {template_file.exists()}")
        print(f"  CSS: {css_file.exists()}")
        print(f"  JS: {js_file.exists()}")
        return False

    # Find all version directories
    version_dirs = [d for d in build_dir.iterdir() if d.is_dir() and not d.name.startswith('_')]

    print(f"Found {len(version_dirs)} version directories")

    injected_count = 0
    for version_dir in version_dirs:
        version_name = version_dir.name

        # Copy static files
        static_dir = version_dir / '_static'
        if static_dir.exists():
            try:
                shutil.copy2(css_file, static_dir / 'version-selector.css')
                shutil.copy2(js_file, static_dir / 'version-selector.js')
                print(f"✓ Injected version selector into {version_name}")
                injected_count += 1
            except Exception as e:
                print(f"✗ Failed to inject into {version_name}: {e}")
        else:
            print(f"⚠ Skipping {version_name} (no _static directory)")

    print(f"\n✓ Successfully injected version selector into {injected_count}/{len(version_dirs)} versions")
    return True

if __name__ == '__main__':
    success = inject_version_selector()
    exit(0 if success else 1)
