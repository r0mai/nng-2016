////////////////////////////////////
// Author: KP
// Version: 1.0
////////////////////////////////////


    $( document ).ready(function() {
        init();
		

    });


////////////////////////////////////
//	  	  	   INIT
////////////////////////////////////

	function init() {
        
	};



////////////////////////////////////
//	    	MESSAGE BAR
////////////////////////////////////

    var MBContainer = '#message-bar';

    function openMessageBar(messageText, messageType, messageLifetime) { 
    // Values of messageType: info, warning, success; Values of messageLifetime: duration im ms. In case of string or integer <= 1 it will be infinite 
        if(messageType !== undefined && messageText !== undefined && messageLifetime !== undefined) {
            setToDefaultMessageBar();
            $(MBContainer + ' > div').addClass(messageType);
            $(MBContainer + ' > div').html(messageText);  
            if(!$(MBContainer).hasClass('active')) {
                $(MBContainer).addClass('active');    
            }
            if(typeof (messageLifetime) === 'number')  {
                if(messageLifetime > 0) {
                    setTimeout(closeMessageBar, messageLifetime);                       
                }

            } 

        }
    }
    function closeMessageBar() {
        $(MBContainer).removeClass('active');
        setTimeout(setToDefaultMessageBar, 400);
    }
    function setToDefaultMessageBar() {
        $(MBContainer + ' > div').removeClass();
        $(MBContainer + ' > div').html('');       
    }
    
