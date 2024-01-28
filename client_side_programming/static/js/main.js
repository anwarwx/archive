window.addEventListener("load", initialize);

function initialize() {
    const mode = document.querySelector(".mode");

    let load = window.localStorage.THEME;
    if (load) document.querySelector("link").href = load;

    mode.addEventListener("click", toggle_style);
}

function toggle_style() {
    if (window.localStorage.THEME === "/main.css") {
        document.querySelector("link").href = "/main.dark.css";
        window.localStorage.THEME = "/main.dark.css";
    } else {
        document.querySelector("link").href = "/main.css";
        window.localStorage.THEME = "/main.css";
    }
}
