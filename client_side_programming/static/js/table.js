window.addEventListener("load", initialize);

function initialize() {
    countdown();
    const delRowButton = document.getElementsByClassName("deleteRowButton");
    for (button of delRowButton) {
        button.addEventListener("click", DeleteRowHandler);
    }
    const setSaleButton = document.getElementById("setSaleButton");
    setSaleButton.addEventListener("click", SetSaleHandler);

    const delSaleButton = document.getElementById("delSaleButton");
    delSaleButton.addEventListener("click", DeleteSaleHandler);
}

function countdown() {
    const dates = document.getElementsByClassName("date");
    const targ = [];
    
    for (const date of dates) {
        if ((date.textContent).length === 0) {
            targ.push("NULL");
        } else {
            let arr = (date.textContent).split('-');
            targ.push(new Date(arr[0], arr[1]-1, arr[2]));
        }
    }
    
    const untilArr = document.getElementsByClassName("timeUntil");
    
    for (let i = 0; i < targ.length; i++) {
        const timeUntil = untilArr[i];
        const id = setInterval((target = targ[i])=>{
            if (target === "NULL") return;

            const current = (new Date()).getTime();
            const diff = target - current;

            const objT = {
                get ret2 () { return 60; },
                get ret1 () { return this.ret2 * 60; },
                get ret0 () { return this.ret1 * 24; }
            }
            const S = (diff / 1000);

            if (S < 0) {
                clearInterval(id);
                timeUntil.innerHTML = "PAST";
                return;
            }
            
            let T = [];
            let tmpS = S
            for (let i = 0; i < 3; i++) {
                const property = `ret${i}`;
                let b = Math.floor(tmpS / objT[property]);
                T[i] = b;
                tmpS %= objT[property];
            }
            tmpS = Math.floor(tmpS);
            
            let str = [];
            const arr = ['d', 'h', 'm']; let i = 0;
            for (v of T) {
                (v) && str.push(v + arr[i]); i+=1;
            }
            str.push(tmpS + 's');
            str = str.join(' ');
            
            timeUntil.textContent = str
        }, 1000);
    }
}

function DeleteRowHandler() {
    async function Delete(row) {
        const URL = "http://localhost:4131";
        const msg = {id: row.id};
        await fetch(`${URL}/api/contact`,
        {
            method: "DELETE",
            headers: {'Content-Type': "application/json"},
            body: JSON.stringify(msg)
        });
    }
    Delete(this.parentNode.parentNode);
    this.parentNode.parentNode.remove();
}

function SetSaleHandler() {
    async function Set(msg) {
        const URL = "http://localhost:4131";
        const message = {message: msg};
        await fetch(`${URL}/api/sale`,
        {
            method: "POST",
            headers: {'Content-Type': "application/json"},
            body: JSON.stringify(message)
        });
    }
    const div = this.parentNode.parentNode.parentNode
    Set(div.firstChild.children[1].value);
}

function DeleteSaleHandler() {
    async function Delete() {
        const URL = "http://localhost:4131";
        await fetch(`${URL}/api/sale`,
        {
            method: "DELETE",
            headers: {'Content-Type': "application/json"},
        });
    }
    Delete();
}
