// calculating coolness level on a scale 0-100,
// based on choice of browser, and subscription

const placeholder = document.getElementById("placeholder");
const calculation = document.getElementById("calculation");

const SUBSCRIBE_WEIGHT = 60;
const CHROME_WEIGHT = 10;
const EDGE_WEIGHT = 30;
const FIREFOX_WEIGHT = 40;
const SAFARI_WEIGHT = 0;
const OPERA_WEIGHT = 20;
let coolness = 0;

const subscribe = document.getElementById("checkbox");
subscribe.checked = false;
subscribe.addEventListener("click", ()=>{
    (subscribe.checked) ? coolness += SUBSCRIBE_WEIGHT : coolness -= SUBSCRIBE_WEIGHT;
    if (coolness === 0) {
        placeholder.textContent = "";
        calculation.textContent = "";
    }
    else {
        placeholder.textContent = "Coolness: ";
        calculation.textContent = coolness;
    }
});

const option = document.getElementById("dropdown");

let previous = 0;
option.addEventListener("change", ()=>{
    const browsers = {
        "Chrome": CHROME_WEIGHT, 
        "Edge": EDGE_WEIGHT,
        "Firefox": FIREFOX_WEIGHT,
        "Safari": SAFARI_WEIGHT,
        "Opera": OPERA_WEIGHT
    }
    const choice = option.selectedOptions[0].textContent;

    coolness -= previous;
    
    if (choice === option[0].textContent) {
        if (coolness === 0) {
            placeholder.textContent = "";
            calculation.textContent = "";
        } else {
            placeholder.textContent = "Coolness: ";
            calculation.textContent = coolness;
        }
        previous = 0;
    }

    for (const browser in browsers) {
        if (choice === browser) {
            coolness += browsers[browser];
            previous = browsers[browser];
            placeholder.textContent = "Coolness: ";
            calculation.textContent = coolness;
        }
    }
});
