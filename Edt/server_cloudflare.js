/*
export default {
    async fetch(request) {

        const response = await fetch(
            "https://edt.iut-velizy.uvsq.fr/Home/GetCalendarData",
            {
                method: "POST",

                headers: {
                    "Content-Type":
                        "application/x-www-form-urlencoded; charset=UTF-8"
                },

                body:
                    "start=2026-09-07" +
                    "&end=2026-09-12" +
                    "&resType=103" +
                    "&calView=agendaWeek" +
                    "&federationIds%5B%5D=GEII1-TDB1" +
                    "&colourScheme=3"
            }
        );

        return new Response(
            await response.text(),
            {
                status: response.status,
                headers: {
                    "Content-Type": "application/json",
                    "Access-Control-Allow-Origin": "*"
                }
            }
        );
    }
};*/

export default {
    async fetch(request) {

        const url = new URL(request.url);

        const start = url.searchParams.get("start") || "2026-09-07";
        const end = url.searchParams.get("end") || "2026-09-12";
        const group = url.searchParams.get("group") || "GEII1-TDB1";

        const params = new URLSearchParams();

        params.append("start", start);
        params.append("end", end);
        params.append("resType", "103");
        params.append("calView", "agendaWeek");
        params.append("federationIds[]", group);
        params.append("colourScheme", "3");

        const response = await fetch(
            "https://edt.iut-velizy.uvsq.fr/Home/GetCalendarData",
            {
                method: "POST",

                headers: {
                    "Content-Type":
                        "application/x-www-form-urlencoded; charset=UTF-8"
                },

                body: params.toString()
            }
        );

        return new Response(
            await response.text(),
            {
                status: response.status,
                headers: {
                    "Content-Type": "application/json",
                    "Access-Control-Allow-Origin": "*"
                }
            }
        );
    }
};
