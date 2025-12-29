document.addEventListener('DOMContentLoaded', function() {
    const vulnerabilities = {
        critical: [],
        warning: [],
        info: []
    };

    const vulnSections = document.querySelectorAll('.severity-critical, .severity-warning, .severity-info');

    vulnSections.forEach((section, index) => {
        let severity = 'info';
        if (section.classList.contains('severity-critical')) {
            severity = 'critical';
        } else if (section.classList.contains('severity-warning')) {
            severity = 'warning';
        }

        const h4 = section.querySelector('h4');
        if (!h4) return;

        const vulnId = `vuln-${severity}-${vulnerabilities[severity].length}`;
        section.id = vulnId;

        let title = h4.textContent.trim();

        vulnerabilities[severity].push({
            id: vulnId,
            title: title,
            section: section
        });
    });

    const totalCount = vulnerabilities.critical.length +
        vulnerabilities.warning.length +
        vulnerabilities.info.length;

    if (totalCount === 0) return;

    const firstVulnSection = document.querySelector('.vulnerability-section');
    if (!firstVulnSection) return;

    const tableContainer = document.createElement('div');
    tableContainer.className = 'vulnerability-table';

    const header = document.createElement('h3');
    header.innerHTML = '📋 Récapitulatif des vulnérabilités';
    tableContainer.appendChild(header);

    const summaryDiv = document.createElement('div');
    summaryDiv.className = 'vulnerability-table-summary';

    // Badge Critical
    if (vulnerabilities.critical.length > 0) {
        const criticalBadge = document.createElement('div');
        criticalBadge.className = 'severity-badge critical';
        criticalBadge.innerHTML = `
            <div class="count">${vulnerabilities.critical.length}</div>
            <div class="label">Critique${vulnerabilities.critical.length > 1 ? 's' : ''}</div>
        `;
        summaryDiv.appendChild(criticalBadge);
    }

    // Badge Warning
    if (vulnerabilities.warning.length > 0) {
        const warningBadge = document.createElement('div');
        warningBadge.className = 'severity-badge warning';
        warningBadge.innerHTML = `
            <div class="count">${vulnerabilities.warning.length}</div>
            <div class="label">Avertissement${vulnerabilities.warning.length > 1 ? 's' : ''}</div>
        `;
        summaryDiv.appendChild(warningBadge);
    }

    // Badge Info
    if (vulnerabilities.info.length > 0) {
        const infoBadge = document.createElement('div');
        infoBadge.className = 'severity-badge info';
        infoBadge.innerHTML = `
            <div class="count">${vulnerabilities.info.length}</div>
            <div class="label">Information${vulnerabilities.info.length > 1 ? 's' : ''}</div>
        `;
        summaryDiv.appendChild(infoBadge);
    }

    tableContainer.appendChild(summaryDiv);

    function createVulnList(severity, items, severityLabel, icon) {
        if (items.length === 0) return null;

        const section = document.createElement('div');
        section.className = `vulnerability-list-section ${severity}`;

        const sectionTitle = document.createElement('h4');
        sectionTitle.innerHTML = `${icon} ${severityLabel} (${items.length})`;
        section.appendChild(sectionTitle);

        const list = document.createElement('ul');
        list.className = 'vulnerability-list';

        items.forEach((vuln, idx) => {
            const li = document.createElement('li');
            li.className = severity;

            const link = document.createElement('a');
            link.href = `#${vuln.id}`;
            link.onclick = (e) => {
                e.preventDefault();
                const target = document.getElementById(vuln.id);
                if (target) {
                    target.scrollIntoView({ behavior: 'smooth', block: 'start' });
                    // Highlight temporaire
                    target.style.transition = 'background 0.3s';
                    const originalBg = target.style.background;
                    target.style.background = 'rgba(231, 76, 60, 0.3)';
                    setTimeout(() => {
                        target.style.background = originalBg;
                    }, 1000);
                }
            };

            link.innerHTML = `
                <span class="severity-icon">${icon}</span>
                <span class="vuln-title">${idx + 1}. ${vuln.title}</span>
                <span class="vuln-severity-label ${severity}">${severityLabel}</span>
            `;

            li.appendChild(link);
            list.appendChild(li);
        });

        section.appendChild(list);
        return section;
    }

    const criticalList = createVulnList('critical', vulnerabilities.critical, 'Critique', '');
    if (criticalList) tableContainer.appendChild(criticalList);

    const warningList = createVulnList('warning', vulnerabilities.warning, 'Avertissement', '');
    if (warningList) tableContainer.appendChild(warningList);

    const infoList = createVulnList('info', vulnerabilities.info, 'Information', '');
    if (infoList) tableContainer.appendChild(infoList);

    firstVulnSection.parentNode.insertBefore(tableContainer, firstVulnSection);

    // Chercher le titre "FAILLES ET VULNÉRABILITÉS"
    const headings = document.querySelectorAll('h2, h3');
    for (let heading of headings) {
        if (heading.textContent.includes('FAILLES') || heading.textContent.includes('VULNERABILIT')) {
            const badge = document.createElement('span');
            badge.className = 'vulnerability-counter';
            badge.textContent = `${totalCount} total`;
            heading.appendChild(badge);
            break;
        }
    }

    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function (e) {
            const href = this.getAttribute('href');
            if (href === '#') return;

            const target = document.querySelector(href);
            if (target) {
                e.preventDefault();
                target.scrollIntoView({ behavior: 'smooth', block: 'start' });
            }
        });
    });
});