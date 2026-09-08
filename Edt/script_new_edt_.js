

const WORKER = "https://little-dream-36b9.argentropcher-me.workers.dev/";


/*
 * ============================================
 * État du programme
 * ============================================
 */

let currentDate = new Date();

let loadedWeekStart = null;
let loadedWeekEnd = null;

let events = [];

let loadedGroup = "";


/*
 * ============================================
 * Utilitaires dates
 * ============================================
 */

function dateOnly(date) {
    return new Date(
        date.getFullYear(),
        date.getMonth(),
        date.getDate()
    );
}


function getMonday(date) {

    const d = dateOnly(date);

    const day = d.getDay();

    /*
     * dimanche = 0
     * lundi = 1
     */

    const diff = day === 0 ? -6 : 1 - day;

    d.setDate(d.getDate() + diff);

    return d;
}


function addDays(date, days) {

    const d = new Date(date);

    d.setDate(d.getDate() + days);

    return d;
}


function formatDateAPI(date) {

    const y = date.getFullYear();

    const m = String(date.getMonth() + 1).padStart(2, "0");

    const d = String(date.getDate()).padStart(2, "0");

    return `${y}-${m}-${d}`;
}


/*
 * ============================================
 * Format affichage
 * ============================================
 */

function formatDateFrench(date) {

    return date.toLocaleDateString(
        "fr-FR",
        {
            weekday: "long",
            day: "numeric",
            month: "long"
        }
    );
}


function formatTime(date) {

    return date.toLocaleTimeString(
        "fr-FR",
        {
            hour: "2-digit",
            minute: "2-digit"
        }
    );
}


/*
 * ============================================
 * Nettoyage description
 * ============================================
 */

function cleanDescription(description) {

    if (!description)
        return "";

    let text = description;

    /*
     * Décodage approximatif des entités HTML
     */

    const textarea = document.createElement("textarea");

    textarea.innerHTML = text;

    text = textarea.value;

    /*
     * <br> devient séparateur
     */

    text = text.replace(/<br\s*\/?>/gi, "\n");

    /*
     * autres balises
     */

    text = text.replace(/<[^>]*>/g, "");

    /*
     * retours multiples
     */

    text = text
        .replace(/\r/g, "")
        .split("\n")
        .map(x => x.trim())
        .filter(x => x.length > 0);

    return text;
}


/*
 * ============================================
 * Analyse d'un événement
 * ============================================
 */
 
function fixEncoding(text) {

    if (!text)
        return "";

    return text
        .replaceAll("â€™", "'")
        .replaceAll("â€˜", "'")
        .replaceAll("â€œ", '"')
        .replaceAll("â€", '"')
        .replaceAll("â€“", "–")
        .replaceAll("â€”", "—")
        .replaceAll("â€¦", "…");
}

function parseEvent(event) {

    const lines = cleanDescription(fixEncoding(event.description));

    return {

        start: new Date(event.start),

        end: new Date(event.end),

        teacher: lines[0] || "",

        group: lines[1] || "",

        room: lines[2] || "",

        subject: lines[3] || "",

        color: event.backgroundColor || "#888",

        textColor: event.textColor || "",

        category: event.eventCategory || "",

        site: event.sites?.join(", ") || "",

        module: event.modules?.join(", ") || ""

    };
}


/*
 * ============================================
 * Charger une semaine
 * ============================================
 */

async function loadWeek(date) {

    const weekStart = getMonday(date);

    const weekEnd = addDays(weekStart, 5);

    const group =
        document.getElementById("group").value.trim();

    if (!group) {

        showError("Veuillez entrer un groupe.");

        return;
    }

    document.getElementById("status").textContent = "Chargement...";


    const params = new URLSearchParams({

        start: formatDateAPI(weekStart),

        end: formatDateAPI(weekEnd),

        group: group

    });


    /*
     * IMPORTANT :
     * On retourne la Promise de fetch.
     * Ainsi loadWeek() attend réellement
     * la fin de la requête.
     */

    return fetch(
        WORKER + "?" + params.toString()
    )

    .then(async response => {

        console.log("STATUS :", response.status);

        if (!response.ok)
            throw new Error("HTTP " + response.status);

        return response.json();
    })
    .then(data => {

        console.log("DATA :", data);

        /*
         * Vérification du JSON
         */

        if (!Array.isArray(data))
            throw new Error("Réponse invalide");


        /*
         * Conversion des événements
         */

        events = data
            .map(parseEvent)
            .filter(event =>
                !isNaN(event.start.getTime()) &&
                !isNaN(event.end.getTime())
            );


        /*
         * La semaine est maintenant réellement chargée.
         */

        loadedWeekStart = weekStart;

        loadedWeekEnd = weekEnd;

        loadedGroup = group;


        /*
         * Aucun événement
         */

        if (events.length === 0) {

            document.getElementById("status").innerHTML =
                `<span class="error">
                    Groupe inconnu ou aucun cours.
                 </span>`;

            render();

            return;
        }


        /*
         * Tout est chargé.
         */

        document.getElementById("status").textContent =
            `${events.length} événements chargés`;


        render();
    })

    .catch(error => {

        console.error("ERREUR :", error);

        events = [];

        loadedWeekStart = null;
        loadedWeekEnd = null;

        document.getElementById("status").innerHTML =
            `<span class="error">
                Groupe inconnu ou réponse invalide.
             </span>`;

        render();
    });
}


/*
 * ============================================
 * Vérifie si une date appartient à la semaine
 * ============================================
 */

function isDateLoaded(date) {

    if (!loadedWeekStart || !loadedWeekEnd)
        return false;

    return (
        date >= loadedWeekStart &&
        date <= loadedWeekEnd
    );
}


/*
 * ============================================
 * Affichage
 * ============================================
 */

function render() {

    document.getElementById("date").textContent =
        formatDateFrench(currentDate);


    const planning =
        document.getElementById("planning");

    planning.innerHTML = "";


    const dayStart = dateOnly(currentDate);

    const dayEnd = addDays(dayStart, 1);


    /*
     * événements du jour
     */

    const dayEvents = events
        .filter(event =>
            event.start >= dayStart &&
            event.start < dayEnd
        )
        .sort((a, b) =>
            a.start - b.start
        );


    if (dayEvents.length === 0) {

        planning.innerHTML =
            `<div class="empty">
                Aucun événement aujourd'hui.
             </div>`;

        return;
    }


    const container =
        document.createElement("div");

    container.className = "day";


    for (const event of dayEvents) {

        const element =
            document.createElement("div");

        element.className = "event";

        element.style.setProperty(
            "--event-color",
            event.color
        );


        /*
         * Vérification événement actuel
         */

        const now = new Date();

        if (
            now >= event.start &&
            now <= event.end
        ) {

            element.classList.add("current");
        }


        /*
         * Heure
         */

        const time =
            document.createElement("div");

        time.className = "event-time";

        time.textContent =
            `${formatTime(event.start)} – ${formatTime(event.end)}`;


        /*
         * Matière
         */

        const title =
            document.createElement("div");

        title.className = "event-title";

        title.textContent = fixEncoding(event.subject) || "Événement";


        /*
         * Informations
         */

        const info =
            document.createElement("div");

        info.className = "event-info";


        const lines = [];

        if (event.teacher)
            lines.push("👤 " + fixEncoding(event.teacher));

        if (event.group)
            lines.push("👥 " + fixEncoding(event.group));

        if (event.room)
            lines.push("📍 " + fixEncoding(event.room));

        if (event.site)
            lines.push("🏢 " + fixEncoding(event.site));

        if (event.module)
            lines.push("📚 " + fixEncoding(event.module));


        info.innerHTML =
            lines
                .map(x => escapeHTML(x))
                .join("<br>");


        /*
         * Catégorie
         */

        const category =
            document.createElement("div");

        category.className =
            "event-category";

        category.textContent =
            fixEncoding(event.category);


        element.appendChild(time);

        element.appendChild(title);

        element.appendChild(info);

        if (event.category)
            element.appendChild(category);


        container.appendChild(element);
    }


    planning.appendChild(container);
}


/*
 * ============================================
 * Protection HTML
 * ============================================
 */

function escapeHTML(text) {

    return String(text)
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
		.replaceAll("'", "&#039;");
}


/*
 * ============================================
 * Erreur
 * ============================================
 */

function showError(message) {

    document.getElementById("status").innerHTML =
        `<span class="error">${escapeHTML(message)}</span>`;
}


/*
 * ============================================
 * Navigation
 * ============================================
 */

document.getElementById("previous")
    .addEventListener("click", async () => {

        currentDate =
            addDays(currentDate, -1);

        /*
         * Si on sort de la semaine chargée,
         * on recharge.
         */

        if (!isDateLoaded(currentDate)) {

            await loadWeek(currentDate);

        } else {

            render();

        }
    });


document.getElementById("next")
    .addEventListener("click", async () => {

        currentDate =
            addDays(currentDate, 1);

        if (!isDateLoaded(currentDate)) {

            await loadWeek(currentDate);

        } else {

            render();

        }
    });


/*
 * ============================================
 * Changement de groupe
 * ============================================
 */

document.getElementById("reload")
    .addEventListener("click", async () => {

        /*
         * Force le rechargement
         */

        loadedWeekStart = null;

        loadedWeekEnd = null;

        events = [];

        loadWeek(currentDate);
    });


document.getElementById("group")
    .addEventListener("keydown", event => {

        if (event.key === "Enter") {

            document
                .getElementById("reload")
                .click();
        }
    });


/*
 * ============================================
 * Thème
 * ============================================
 */

document.getElementById("theme")
    .addEventListener("click", () => {

        document.body.classList.toggle("dark");

        document.getElementById("theme").textContent =
            document.body.classList.contains("dark")
                ? "☀"
                : "☾";
    });


/*
 * ============================================
 * Démarrage
 * ============================================
 */

loadWeek(currentDate);

