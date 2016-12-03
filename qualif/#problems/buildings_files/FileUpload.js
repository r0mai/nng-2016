var FileUpload = (function(){
	
	var group;

	function init(options)
	{
		group = $(options.group);

		addEvents();
	}


	function addEvents()
	{
		group.each(function(index, el) {
			$('input[type="file"]', el).change(function(event) {
				$('.file-name', el).text(this.value);
			});		
		});	
	}

	return{
		init:init
	}

})();