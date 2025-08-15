function updateTime() {
    fetch('/time')
        .then(r => r.json())
        .then(data => {
            document.getElementById('currentTime').textContent = data.time + ' ' + data.date;
        })
        .catch(err => {
            document.getElementById('currentTime').textContent = 'Error loading time';
        });
}

function updateSystemStatus() {
    fetch('/status')
        .then(r => r.json())
        .then(data => {
            document.getElementById('freeMemory').textContent = data.freeMemory;
            document.getElementById('wifiStatus').textContent = data.wifi;
        })
        .catch(err => {
            document.getElementById('freeMemory').textContent = 'Error';
            document.getElementById('wifiStatus').textContent = 'Error';
        });
}

function loadDisplaySettings() {
    fetch('/display/settings')
        .then(r => r.json())
        .then(data => {
            document.getElementById('brightness').value = data.brightness;
            document.getElementById('scrollSpeed').value = data.scrollSpeed;
            document.getElementById('scrollDirection').value = data.scrollDirection;
            document.getElementById('panelType').value = data.panelType;
            document.getElementById('fontType').value = data.fontType;
            document.getElementById('animationType').value = data.animationType;
            document.getElementById('animationEnabled').checked = data.animationEnabled;
        })
        .catch(err => showStatus('Error loading display settings', 'error'));
}

function updateDisplaySettings() {
    const settings = {
        brightness: parseInt(document.getElementById('brightness').value),
        scrollSpeed: parseInt(document.getElementById('scrollSpeed').value),
        scrollDirection: parseInt(document.getElementById('scrollDirection').value),
        panelType: parseInt(document.getElementById('panelType').value),
        fontType: parseInt(document.getElementById('fontType').value),
        animationType: parseInt(document.getElementById('animationType').value),
        animationEnabled: document.getElementById('animationEnabled').checked
    };
    
    fetch('/display/settings', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(settings)
    })
    .then(r => r.text())
    .then(msg => showStatus(msg, 'success'))
    .catch(err => showStatus('Error updating display settings', 'error'));
}

function updateScrollContent() {
    const contents = {
        time: document.getElementById('content_time').checked,
        date: document.getElementById('content_date').checked,
        rss: document.getElementById('content_rss').checked,
        quotes: document.getElementById('content_quotes').checked,
        facts: document.getElementById('content_facts').checked
    };
    
    fetch('/display/content', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(contents)
    })
    .then(r => r.text())
    .then(msg => showStatus(msg, 'success'))
    .catch(err => showStatus('Error updating scroll content', 'error'));
}

function setManualTime() {
    const datetime = document.getElementById('manualDateTime').value;
    if (!datetime) {
        showStatus('Please select a date and time', 'error');
        return;
    }
    
    fetch('/settime', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: 'datetime=' + encodeURIComponent(datetime)
    })
    .then(r => r.text())
    .then(msg => {
        showStatus(msg, 'success');
        updateTime();
    })
    .catch(err => showStatus('Error setting time', 'error'));
}

function updateTimezone() {
    const timezone = document.getElementById('timezone').value;
    
    const settings = {
        tzRegion: timezone
    };
    
    fetch('/settings', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(settings)
    })
    .then(r => r.text())
    .then(msg => {
        showStatus('Timezone updated to ' + timezone, 'success');
        setTimeout(updateTime, 1000);
    })
    .catch(err => showStatus('Error updating timezone', 'error'));
}

function loadFeeds() {
    fetch('/feeds')
        .then(r => r.json())
        .then(feeds => {
            const feedsList = document.getElementById('feedsList');
            feedsList.innerHTML = '';
            
            feeds.forEach((feed, index) => {
                const feedDiv = document.createElement('div');
                feedDiv.className = 'feed-item';
                feedDiv.innerHTML = `
                    <div class="feed-controls">
                        <label>
                            <input type="checkbox" ${feed.enabled ? 'checked' : ''} 
                                   onchange="toggleFeed(${index}, this.checked)">
                            <strong>${feed.name}</strong>
                        </label>
                        <button onclick="removeFeed(${index})" class="remove-btn">Remove</button>
                    </div>
                    <input type="text" value="${feed.url}" 
                           onchange="updateFeedUrl(${index}, this.value)" 
                           placeholder="RSS Feed URL">
                `;
                feedsList.appendChild(feedDiv);
            });
        })
        .catch(err => showStatus('Error loading feeds', 'error'));
}

function toggleFeed(index, enabled) {
    fetch('/feeds')
        .then(r => r.json())
        .then(feeds => {
            feeds[index].enabled = enabled;
            return fetch('/feeds', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(feeds)
            });
        })
        .then(r => r.text())
        .then(msg => showStatus(`Feed ${enabled ? 'enabled' : 'disabled'}`, 'success'))
        .catch(err => showStatus('Error updating feed', 'error'));
}

function updateFeedUrl(index, newUrl) {
    fetch('/feeds')
        .then(r => r.json())
        .then(feeds => {
            feeds[index].url = newUrl;
            return fetch('/feeds', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(feeds)
            });
        })
        .then(r => r.text())
        .then(msg => showStatus('Feed URL updated', 'success'))
        .catch(err => showStatus('Error updating feed URL', 'error'));
}

function addNewFeed() {
    const name = prompt('Enter feed name:');
    if (!name) return;
    
    const url = prompt('Enter RSS feed URL:');
    if (!url) return;
    
    fetch('/feeds')
        .then(r => r.json())
        .then(feeds => {
            feeds.push({name: name, url: url, enabled: true});
            return fetch('/feeds', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(feeds)
            });
        })
        .then(r => r.text())
        .then(msg => {
            showStatus('Feed added successfully', 'success');
            loadFeeds();
        })
        .catch(err => showStatus('Error adding feed', 'error'));
}

function removeFeed(index) {
    if (!confirm('Are you sure you want to remove this feed?')) return;
    
    fetch('/feeds')
        .then(r => r.json())
        .then(feeds => {
            feeds.splice(index, 1);
            return fetch('/feeds', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(feeds)
            });
        })
        .then(r => r.text())
        .then(msg => {
            showStatus('Feed removed', 'success');
            loadFeeds();
        })
        .catch(err => showStatus('Error removing feed', 'error'));
}

function resetFeeds() {
    if (!confirm('Reset all feeds to default? This will remove custom feeds.')) return;
    
    fetch('/feeds/reset', {method: 'POST'})
        .then(r => r.text())
        .then(msg => {
            showStatus(msg, 'success');
            loadFeeds();
        })
        .catch(err => showStatus('Error resetting feeds', 'error'));
}

function loadRSSSettings() {
    fetch('/settings')
        .then(r => r.json())
        .then(data => {
            document.getElementById('fetchInterval').value = data.fetchInterval;
            document.getElementById('maxNewsAgeHours').value = data.maxNewsAgeHours;
            document.getElementById('maxHeadlinesPerFeed').value = data.maxHeadlinesPerFeed;
            document.getElementById('timezone').value = data.tzRegion;
        })
        .catch(err => showStatus('Error loading RSS settings', 'error'));
}

function updateRSSSettings() {
    const settings = {
        fetchInterval: parseInt(document.getElementById('fetchInterval').value),
        maxNewsAgeHours: parseInt(document.getElementById('maxNewsAgeHours').value),
        maxHeadlinesPerFeed: parseInt(document.getElementById('maxHeadlinesPerFeed').value),
        tzRegion: document.getElementById('timezone').value
    };
    
    fetch('/settings', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(settings)
    })
    .then(r => r.text())
    .then(msg => showStatus('RSS settings updated successfully', 'success'))
    .catch(err => showStatus('Error updating RSS settings', 'error'));
}

function fetchRSS() {
    showStatus('Fetching RSS feeds...', 'success');
    fetch('/feeds/fetch', {method: 'POST'})
        .then(r => r.text())
        .then(msg => showStatus('RSS fetch completed: ' + msg, 'success'))
        .catch(err => showStatus('Error fetching RSS', 'error'));
}

function showStatus(message, type) {
    const statusDiv = document.getElementById('status');
    statusDiv.innerHTML = `<div class="status ${type}">${message}</div>`;
    setTimeout(() => statusDiv.innerHTML = '', 3000);
}

// Initialize page
document.addEventListener('DOMContentLoaded', function() {
    updateTime();
    updateSystemStatus();
    loadDisplaySettings();
    loadRSSSettings();
    loadFeeds();
    
    // Update time every 30 seconds
    setInterval(updateTime, 30000);
    
    // Update system status every 60 seconds
    setInterval(updateSystemStatus, 60000);
});
