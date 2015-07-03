var timer;
var step;
var min;
var max;
var _height;
$.fn.extend({
    numberType:function() {
        this.each(function() {
            step = parseFloat($(this).attr("step"));
            min = Number($(this).attr("min"));
            max = parseFloat($(this).attr("max"));
            _height = $(this).height() + 4;
            $(this).css("margin", "0")
            $(this).wrapAll("<div style='height: " + _height + "px;width:" + ($(this).width() + 21) + "px;'></div>");
            $(this).after("<div style='float: right;margin:auto 0;width:17px;height:34px;position: relative;'><a href='javascript:;'><div class='up' onselectstart='return false'>+</div></a>" +
                "<a href='javascript:;'><div class='down' onselectstart='return false'>-</div></a></div>");
   
            var up = $(this).siblings("div").find(".up");
            up.css({"border":"1px #ccc solid","background":"#eaeaea","color":"#aaa","width":"15px","height":"15px","lineHeight":"15px","textAlign":"center","position": "absolute","top":"0px","left":"0px"
            }).bind({
                    mouseenter:function() {
                        $(this).css("background", "#f3f3f3");
                    },
                    mouseleave:function() {
                        $(this).css("background", "#eaeaea");
                        clearInterval(timer);
                    },
                    mousedown:function() {
						$.up_event(this);
                    },
                    mouseup:function() {
                        $(this).css("background", "#eaeaea");
                        clearInterval(timer);
                    }
                });
            var down = $(this).siblings("div").find(".down");
            down.css({"border":"1px #ccc solid","background":"#eaeaea","color":"#aaa","width":"15px","height": "15px","lineHeight":"15px","textAlign":"center","position": "absolute","top":"17px","left":"0px"
            }).bind({
                    mouseenter:function() {
                        $(this).css("background", "#f3f3f3");
                    },
                    mouseleave:function() {
                        $(this).css("background", "#eaeaea");
                        clearInterval(timer);
                    },
                    mousedown:function() {
						$.down_event(this);
                    },
                    mouseup:function() {
                        $(this).css("background", "#eaeaea");
                        clearInterval(timer);
                    }
                });
            $.numeral(this);
        });
    },
});
$.extend({
    up_event:function(_e){
	  $(_e).css("background", "#dddddd");
                        var that = _e;
						
                        $.up(that, {step:step,min:min,max:max});
                        timer = setInterval(function() {
                            $.up(that, {step:step,min:min,max:max});
                        }, 150);
	},
	down_event:function(_e){
	   $(_e).css("background", "#dddddd");
                        var that = _e;
                        $.down(that, {step:step,min:min,max:max});
                        timer = setInterval(function() {
                            $.down(that, {step:step,min:min,max:max});
                        }, 150);
	},
    up:function (_e, _setting) {
        if (isNaN(_setting.step)) {
            _setting.step = 1
        }
        var input = $(_e).parent().parent().siblings("input");
        var num = parseFloat(input.val()) || 0;
        var res = num + _setting.step;
        if (res > _setting.max) {
            input.val(_setting.max);
        } else {
            input.val(res);
        }
    },
    down:function (_e, _setting) {
        if (isNaN(_setting.step)) {
            _setting.step = 1
        }
        var input = $(_e).parent().parent().siblings("input");
        var num = parseFloat(input.val()) || 0;
        var res = num - _setting.step;
        if (res < _setting.min) {
            input.val(_setting.min);
        } else {
            input.val(res);
        }
    },
    numeral : function(_e) {
        $(_e).css("ime-mode", "disabled");
        $(_e).bind("keypress", function(event) {
            var keyCode = event.which;
            if (keyCode == 46) {
                if (_e.value.indexOf(".") != -1) {
                    return false;
                }
            }
            if (keyCode == 45) {
                _e.value = -_e.value;
                return false;
            } else {
                return keyCode >= 45 && keyCode <= 57;
            }
        });
        $(_e).bind("blur", function() {
            if (_e.value.lastIndexOf(".") == (_e.value.length - 1)) {
                _e.value = _e.value.substr(0, _e.value.length - 1);
            } else if (isNaN(_e.value)) {
                _e.value = "";
            }
        });
        $(_e).bind("paste", function() {
            var s = window.clipboardData.getData('text');
            if (!/\D/.test(s));
            value = s.replace(/^0*/, '');
            return false;
        });
        $(_e).bind("dragenter", function() {
            return false;
        });
        $(_e).bind("keyup", function() {
            if (/(^0+)/.test(_e.value))_e.value = _e.value.replace(/^0*/, '');
        });
    }
});


