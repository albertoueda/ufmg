<?php header('Content-Type: text/html; charset=ISO-8859-1'); ?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<meta http-equiv='Content-Type' content='text/html; charset=ISO-8859-1'>
<script src="jquery-1.8.3.min.js" ></script>
<script src="jquery.blockUI.js" ></script>
<script src="bootstrap/js/bootstrap.min.js"></script>
<script type='text/javascript' src='https://www.google.com/jsapi'></script>

<link rel="stylesheet" type="text/css" href="search.css" /> 

<link href="bootstrap/css/bootstrap.min.css" rel="stylesheet" media="screen">

<link href="bootstrap/css/bootstrap-responsive.css" rel="stylesheet">
<title>Latin Search Engine</title>
</head>

<body>    
	<br><br>
	<h1 style="font-family:Palatino Linotype, serif" align="center">
	Latin Search Engine
	</h1>
    <div class="container" style="margin-top:30px; margin-left:10px; text-align:center">

	<form class="form-search input-append" style="margin-bottom:20px">	
		<input style="margin-left:50px; padding-left:10px; font-size: 25px" type="text" name="queryText" id="queryText" size=45 maxlength="100" 
		    value="" class="input-search" placeholder="">
		<button type="search" name="sub" class="btn" id="sub" 
		    style="margin-left:20px; margin-top:4px;font-size: 20px; padding:10px">Search</button>
<!--	    
-->			
			<div class="bs-docs-example ">
				 <label class="checkbox">
						<input value="0" type="checkbox"> Boolean&nbsp;
			     </label>
			      <label class="checkbox">
						<input value="1" type="checkbox" checked="true"> Cosine&nbsp;
			      </label>
			       <label class="checkbox">
						<input value="2" type="checkbox"> BM25&nbsp;
			       </label>
			    <!--
				<label class="checkbox">
						<input value="3" type="checkbox" > Mix (Cosine+BM25)&nbsp;
				</label>
				-->
				<label class="checkbox">
						<input value="4" type="checkbox" > Cosine (Anchor)&nbsp;
				 </label>
				 <!--
				 <label class="checkbox">
						<input value="5" type="checkbox" > BM25 (Anchor)&nbsp;
				 </label>
				 -->
				  <label class="checkbox">
						<input value="6" type="checkbox" > Pagerank&nbsp;
				 </label>
				 <!--
				 <label class="checkbox">
						<input value="7" type="checkbox" > Mix (All)
				 </label>
				 -->
			</div>		
			</div>
	</form>

	<div id=queryResult style="margin-left:20px;"></div>
	
    </div>
    
	<p style="position:fixed; right:20px; bottom:10px; width:100%; text-align:right"><i>(Thanks to Itamar Hata)</i></p>
</body>

    <script>
	
    google.load('visualization', '1', {packages:['corechart', 'gauge']});
//    google.load('visualization', '1', {packages:['gauge']});

	jQuery('#queryText').focus();

	
      function drawGauge(table, idDiv) {
        var data = google.visualization.arrayToDataTable(table);

        var options = {
          width: 400, height: 120,
	  redFrom: 0, redTo: 10,
          yellowFrom:10, yellowTo: 30,
	  greenFrom: 30, greenTo :  100,
          minorTicks: 5
        };

        var chart = new google.visualization.Gauge(document.getElementById(idDiv));
        chart.draw(data, options);
      }
      
      function drawChart(table, idDiv) {
	 var data = google.visualization.arrayToDataTable(table);

	    var options = {
	      title: 'Recall vs. Precision',
	      hAxis: {title: 'Recall', minValue: 0, maxValue: 100},
	      vAxis: {title: 'Precision', minValue: 0, maxValue: 100},
	      legend: 0
	    };

        var chart = new google.visualization.LineChart(document.getElementById(idDiv));
        chart.draw(data, options);
      }
	
	function getModels(){
		models = []
		jQuery("input:checkbox").filter(":checked").each(function(i){
			models[i] = $(this).val();
		});
		
		if(models.length == 0){
			$("#alertModel").show();
			$("#alertModel").alert();
		}else{
			$("#alertModel").hide();

		}
		
		return models;
	}
	
	function searchPages(page){
	
		if(!page){
			page = 0;
		}
		models = getModels();
		if(models.length){
			jQuery('#sub').disableButton();	
			jQuery.blockUI({
					message: '<img width="50px" src="images/loading.gif" />'
					 // message: '<img src="images/loading.gif"/>'
					}); 
			jQuery.ajax({
				type: 'GET',
				url: "query.php",
				cache: false,
				contentType: "text/html; charset=ISO-8859-1",
				data : {
					queryText: jQuery('#queryText').val(),
					models: models.join(),
					page: page
				}
				}).done(function( html ) {
					jQuery.unblockUI();
					jQuery('#sub').enableButton();
					jQuery("#queryResult").html(html);
					jQuery('#statsId2').html(jQuery('#statsId').html());
					jQuery('#statsId').html('');
					jQuery('#queryText').focus();
					
			});
		}
	}
	
		
    jQuery.fn.enableButton = function () {
		jQuery(this).removeClass('buttonDisabled');
		jQuery(this).addClass('buttonEnabled');
		jQuery(this).attr('disabled', false);
    };

    jQuery.fn.disableButton = function () {
		jQuery(this).removeClass('buttonEnabled');
		jQuery(this).removeClass('buttonHover');
		jQuery(this).addClass('buttonDisabled');
		jQuery(this).attr('disabled', true);
    };
    
    jQuery(document).ready(function() {
		jQuery("#alertModel").hide();
		jQuery('.dropdown-toggle').dropdown();
	    jQuery('#queryText').keypress(function(e) {
		    if(e.which == 13) {
			jQuery(this).blur();
			searchPages();
		    }
	     });

	    jQuery('#sub').enableButton();
	    
	    jQuery("input:checkbox").click(function(){
		    if( getModels().length != 0 &&  jQuery('#queryText').val() != "" ){
			searchPages();
		    }
		});
	    
	    jQuery('.queries').click( function() { 
				jQuery('#queryText').val(jQuery(this).text());
				searchPages();
				//return false; 
			} 
		);

	    jQuery('#sub').click(function(event) {
			searchPages();
		return false;
	    });

    });

     jQuery(document).ajaxError(function (event, request, settings) {
	     jQuery.unblockUI();
	     jQuery('#sub').enableButton();
	     s = '';
	     for(b in event){
			 s += b + ' = ' + event[b] + '\n';
		 }
	     alert('ERROR ' + s);
     });

     jQuery(document).ajaxStop(jQuery.unblockUI); 

    </script>


</html>
