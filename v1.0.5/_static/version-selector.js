/* Version selector JavaScript for sphinx-multiversion */

document.addEventListener('DOMContentLoaded', function() {
    var versionSelector = document.querySelector('.rst-versions');

    if (versionSelector) {
        var currentVersion = versionSelector.querySelector('.rst-current-version');

        currentVersion.addEventListener('click', function() {
            versionSelector.classList.toggle('shift-up');
        });

        // Close version selector when clicking outside
        document.addEventListener('click', function(event) {
            if (!versionSelector.contains(event.target)) {
                versionSelector.classList.remove('shift-up');
            }
        });
    }
});
