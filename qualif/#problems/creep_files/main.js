

$(window).on('load', function (e) {
    $('body').show();
});

/* PJAX */

$.ajaxSetup({ cache: false });

$(document).pjax('a[data-pjax]', '#pjax-container', {timeout:10000 });

//$(document).pjax('form[data-pjax]', '#pjax-container', {timeout:10000, push:false});

$(document).on('pjax:send', function() {
  $('#preloader').show();
  $('.overlay').show();
});

$(document).on('pjax:complete', function() {
  $('#preloader').hide();
  $('.overlay').hide(); 
});


$(document).on('submit', 'form[data-pjax]', function(event) {
  $.pjax.submit(event, '#pjax-container')
})


$(document).on('pjax:end', function() {
	
});

$(document).on('pjax:error', function(event, xhr, textStatus, errorThrown, options){
	
	console.log(event, xhr, textStatus, errorThrown, options)

});



