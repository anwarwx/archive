window.addEventListener("load", ()=>{ setInterval(UpdateListener, 1000); });

async function UpdateListener() {
    const div = document.getElementById("sale_banner");
    const span = div.firstChild;

    const URL = "http://localhost:4131";
    const response = await fetch(`${URL}/api/sale`);
    if (!response.ok) return;
    const json = await response.json();

    if (!json.message) {
        div.style.visibility = "hidden";
        span.textContent = "";
        return;
    }

    span.textContent = json.message;
    span.style.marginLeft =
        ((div.offsetWidth - span.offsetWidth)/2).toString() + "px";

    if (json.active) div.style.visibility = "visible";
    else div.style.visibility = "hidden";
}
