/* Version selector JavaScript for sphinx-multiversion */

document.addEventListener('DOMContentLoaded', function() {
    console.log('Version selector script loaded');

    var versionSelector = document.querySelector('.rst-versions');
    console.log('Version selector element:', versionSelector);

    if (versionSelector) {
        var currentVersion = versionSelector.querySelector('.rst-current-version');
        console.log('Current version element:', currentVersion);

        if (currentVersion) {
            currentVersion.addEventListener('click', function(event) {
                event.preventDefault();
                event.stopPropagation();
                console.log('Version selector clicked');
                versionSelector.classList.toggle('shift-up');
            });

            // Close version selector when clicking outside
            document.addEventListener('click', function(event) {
                if (!versionSelector.contains(event.target)) {
                    versionSelector.classList.remove('shift-up');
                }
            });

            console.log('Version selector initialized successfully');
        }
    } else {
        console.warn('Version selector element not found');
    }
});
