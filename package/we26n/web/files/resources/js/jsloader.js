function loadjs(url,callback){
	var head = document.getElementsByTagName("head")[0];
	var script = document.createElement('script');
	script.onload = script.onreadystatechange = script.onerror = function (){
		if (script && script.readyState && /^(?!(?:loaded|complete)$)/.test(script.readyState)) return;
		script.onload = script.onreadystatechange = script.onerror = null;
		script.src = '';
		script.parentNode.removeChild(script);
		script = null;
		callback();
	}
	script.charset = "utf-8";
	script.src = url;
	try {
		head.appendChild(script);
	} catch (exp) {}
}

function loadmultijs(url,callback){
	if(Object.prototype.toString.call(url)==='[object Array]'){	//是否数组
		this.suc = 0;			//加载计数
		this.len = url.length;	//数组长度
		var a = this;
		for(var i = 0;i < url.length;i++){
			loadjs(url[i],function(){ a.suc++; if(a.suc == a.len) try{callback();}catch(e){} });
		}
	}else if(typeof(url) == 'string'){
		loadjs(url,callback);
	}
}
