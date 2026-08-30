<h1>Bienvenue sur le test de localisation depuis win32<br>pour récupérer la localisation d'une machine sur windows (v 10 minimum) avec winRT implémenté dans du code C brut.</h1>
<br>
<h2>localisationwin32v1.c et localisationwin32v1.exe est le premier essai fonctionnel (avec débogage).</h2>
<h2>localisationwin32v2.c et localisationwin32v2.dll est le code réduit en .dll pour une application win32 autre à implémentation facile.</h2>
<br>
<h2>Fonction de la dll (toujours appeler int geo_process(void) en premier)</h2>
<ul>
<li><strong>int geo_process(void)</strong> = effectue la requête de la géolocalisation. renvoie 0 en cas d'échec, sinon 1.<br>(peut être appeler pour réactualisé les données)</li>
<li><strong>HRESULT geo_get_hr(void)</strong> = en cas d'échec, le code d'erreur Microsoft est renvoyé ici</li>
<li><strong>const char *geo_get_error(void)</strong> = en cas d'échec, renvoie le message d'erreur en char * (2048 caractères max).</li>
<li><strong>double geo_get_latitude(void)</strong> = renvoie la latitude en double.</li>
<li><strong>double geo_get_longitude(void)</strong> = renvoie la longitude en double.</li>
<li><strong>double geo_get_accuracy(void)</strong> = renvoie la précision en double.<br>(actuellement, pourrait être casté en int car ne renvoie que la valeur entière pour moi)</li>
<li><strong>int geo_view(void)</strong> = renvoie les informations ou l'erreur de manière simplifié dans la console.</li>
</ul>
<br>
<h2>Attention, pour utilisé cette dll, il faut avoir un compte Microsoft actif sur l'appareil.<br>La dll ne prend pas du multithread en charge, appeler sur simple thread pour toutes les requêtes.</h2>
<br>

> [!TIP]  <h2>localisationwin32v2.dll peut être utiliser avec fr-simplecodeX.X.X.<br>Exemple : geolocation.frc</h2>
