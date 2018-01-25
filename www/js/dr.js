$(function(){ 
	function DoRequest(path, arg, succ, err, comp) {
		$.ajax({
			url : path,
			type : 'POST',
			async: true,
			data: arg,
			timeout : 5000,
			//dataType: 'json',
			//dataType: 'html',
			dataType: 'html',
			beforeSend: function(xhr) {
			},
			success: function(data, textStatus, jqXHR) {
				succ(data);
			},
			error: function(xhr, textStatus) {
				err(textStatus);
			},
			complete: function(xhr, textStatus) {
				comp(textStatus);
			}
		});
	}


	$("#include").click(function() {
		var clickObj = $(this);
		var btntxt = clickObj.html();

		var arg  = {};

		DoRequest("/include", arg, function(data) { //success
				//console.log("response:" + data);
		}, function(stat) { //error
			console.log("Requst /next Error:" + stat);
		}, function(stat) {
			console.log("Requst /next Complete!");
		});
	});

	$("#exclude").click(function() {
		var clickObj = $(this);
		var btntxt = clickObj.html();

		var arg  = {};

		DoRequest("/exclude", arg, function(data) { //success
				//console.log("response:" + data);
		}, function(stat) { //error
			console.log("Requst /next Error:" + stat);
		}, function(stat) {
			console.log("Requst /next Complete!");
		});
	});
}); 
