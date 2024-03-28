let card1 = document.getElementsByTagName("img")[5];//读取第六个img标签的内容

card1.style.visibility= "hidden";//修改可视化

let card2 = document.getElementsByTagName("img")[6];
card2.src ="./resources/12206212.jpg"//把图片修改为目标图片

function load_txt(name) {
    let xhr = new XMLHttpRequest(),
        okStatus = document.location.protocol === "file:" ? 0 : 200;
    xhr.open('GET', name, false);
    xhr.overrideMimeType("text/html;charset=utf-8");//默认为utf-8
    xhr.send(null);
    return xhr.status === okStatus ? xhr.responseText : null;
}

function card_set(){
    let mess = load_txt("./a.txt");
    timestamp = (new Date()).getTime();
    let pos = mess.substring(0,2);
    let length_card = mess.length;
    let card = document.getElementsByTagName("img")[pos];
    let card_contain = document.getElementsByTagName("a")[pos];
    if(mess[2]==0){
        card.style.visibility= "hidden";

    }
    else{
        if(card.src!=("./resources/"+mess.substring(4,length_card)+".jpg")){

            card.src ="./resources/"+mess.substring(4,length_card)+".jpg"+ '?_=' + timestamp;//把图片修改为目标图片
            card.style.visibility= "visible";
            card_contain.href = "https://www.ourocg.cn/search/" + mess.substring(3,length_card) + "/";

            if(mess[3]==0){
             card.style.transform = 'rotate(90deg)';//旋转90°，此代码需后续移动至其他地方

            }


        }
    }
}

function card(a){
    let mess = a;
    let pos = mess.substring(0,2);
    let length_card = mess.length;
    let card = document.getElementsByTagName("img")[pos];
    let card_contain = document.getElementsByTagName("a")[pos];
    if(mess[2]==0){
        card.style.visibility= "hidden";

    }
    else{
        if(card.src!=("./resources/"+mess.substring(4,length_card)+".jpg")){

            card.src ="./resources/"+mess.substring(4,length_card)+".jpg"+ '?_=' + timestamp;//把图片修改为目标图片
            card.style.visibility= "visible";
            card_contain.href = "https://www.ourocg.cn/search/" + mess.substring(3,length_card) + "/";

            if(mess[3]==0){
             card.style.transform = 'rotate(90deg)';//旋转90°，此代码需后续移动至其他地方

            }


        }
    }
}


setInterval(card_set,3000);

//const ws = new WebSocket('ws://127.0.0.1:8226');

//ws.addEventListener('open', function (event) {
    //socket.send('Hello Server!');
    //console.log('connected');
//});

// Listen for messages
//ws.addEventListener('message', function (event) {
    //console.log('Message from server ', event.data);
//});
//window.onbeforeunload = function (ws.close();) {};



