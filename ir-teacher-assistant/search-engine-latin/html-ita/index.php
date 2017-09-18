<?php header('Content-Type: text/html; charset=ISO-8859-1'); ?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<meta http-equiv='Content-Type' content='text/html; charset=ISO-8859-1'>
<link rel="icon" type="image/gif"  href="images/Scope.gif">
<script src="jquery-1.8.3.min.js" ></script>
<script src="jquery.blockUI.js" ></script>
<script src="bootstrap/js/bootstrap.min.js"></script>
<script type='text/javascript' src='https://www.google.com/jsapi'></script>

<link rel="stylesheet" type="text/css" href="search.css" /> 

<link href="bootstrap/css/bootstrap.min.css" rel="stylesheet" media="screen">

<link href="bootstrap/css/bootstrap-responsive.css" rel="stylesheet">
<title>ItaSearch</title>
</head>


    

<body>    

	<header class="jumbotron subhead" id="overview" >
	  <div class="container">
		<h1>
		ItaSearch <img  src="./images/scope.png" width="45px" height="45px"  alt="logo"/>
		</h1>
	  </div>
	</header>

    <div class="container">

	<form class="form-search input-append">
		<input type="text" name="queryText" id="queryText" size=50  maxlength="300" value="" class="input-search" placeholder="Type your query">
		<button type="search" name="sub" class="btn" id="sub"><i class="icon-search"></i>search</button>
		
		     <div class="btn-group">
			<a class="btn dropdown-toggle" data-toggle="dropdown" href="#">
			Avaliation Queries
			<span class="caret"></span>
			</a>
			<ul class="dropdown-menu">
				<li><a class="queries" href="#">ana maria braga</a></li>
				<li><a class="queries" href="#">baixaki</a></li>
				<li><a class="queries" href="#">caixa economica federal</a></li>
				<li><a class="queries" href="#">casa e video</a></li>
				<li><a class="queries" href="#">claro</a></li>
				<li><a class="queries" href="#">concursos</a></li>
				<li><a class="queries" href="#">detran</a></li>
				<li><a class="queries" href="#">esporte</a></li>
				<li><a class="queries" href="#">frases de amor</a></li>
				<li><a class="queries" href="#">funk</a></li>
				<li><a class="queries" href="#">globo</a></li>
				<li><a class="queries" href="#">gmail</a></li>
				<li><a class="queries" href="#">google</a></li>
				<li><a class="queries" href="#">hotmail</a></li>
				<li><a class="queries" href="#">ig</a></li>
				<li><a class="queries" href="#">jogos de meninas</a></li>
				<li><a class="queries" href="#">jogos online</a></li>
				<li><a class="queries" href="#">mario</a></li>
				<li><a class="queries" href="#">mercado livre</a></li>
				<li><a class="queries" href="#">msn</a></li>
				<li><a class="queries" href="#">naruto</a></li>
				<li><a class="queries" href="#">oi</a></li>
				<li><a class="queries" href="#">orkut</a></li>
				<li><a class="queries" href="#">panico</a></li>
				<li><a class="queries" href="#">poquer</a></li>
				<li><a class="queries" href="#">previsao do tempo</a></li>
				<li><a class="queries" href="#">receita federal</a></li>
				<li><a class="queries" href="#">record</a></li>
				<li><a class="queries" href="#">rio de janeiro</a></li>
				<li><a class="queries" href="#">terra</a></li>
				<li><a class="queries" href="#">uol</a></li>
				<li><a class="queries" href="#">vivo</a></li>
				<li><a class="queries" href="#">yahoo</a></li>
				<li><a class="queries" href="#">youtube</a></li>
			</ul>
			</div>
			
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
				<label class="checkbox">
						<input value="3" type="checkbox" > Mix (Cosine+BM25)&nbsp;
				 </label>
				<label class="checkbox">
						<input value="4" type="checkbox" > Cosine (Anchor)&nbsp;
				 </label>
				 <label class="checkbox">
						<input value="5" type="checkbox" > BM25 (Anchor)&nbsp;
				 </label>
				  <label class="checkbox">
						<input value="6" type="checkbox" > Page Rank&nbsp;
				 </label>
				 <label class="checkbox">
						<input value="7" type="checkbox" > Mix (All)
				 </label>
			</div>
			<div id="alertModel" class="alert alert-block alert-error fade in" style="display:none;">
			<h4 class="alert-heading">You must select at least one model.</h4>
		
			</div>
	</form>

	<div id=queryResult></div>
	
	<img class='scope' src="./images/scope2.png" width="45px" height="45px"  alt="logo"/>

    </div>
    
</body>

<style>
img.scope {
width: 50px;
height:50px;
position:fixed;
z-index: 10000;
cursor: crosshair;
}
</style>

    <script>
	
    google.load('visualization', '1', {packages:['corechart', 'gauge']});
//    google.load('visualization', '1', {packages:['gauge']});

	
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
					 message: '<h1><img src="images/coffe.gif" />Searching...</h1>'
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
					
					if(jQuery('#myTab').length){
						//debugger;
						localGauge = eval($('#localGauge').val());
						globalGauge = eval($('#globalGauge').val());
						localLine = eval($('#localLine').val());
						globalLine = eval($('#globalLine').val());
						
						chartHtml = '<fieldset><legend class="smaller">Performance for query: ' + jQuery('#queryText').val() + '</legend><div class="container-fluid"><div class="row-fluid"><div class="span6"><div id="lLine"></div></div><div class="span6">Average Precision @10<div id="lGauge"></div></div></div></div></fieldset>';
						chartHtml += '<fieldset><legend class="smaller">Average Performance</legend><div class="container-fluid"><div class="row-fluid"><div class="span6"><div id="gLine"></div></div><div class="span6">Mean Average Precision @10<div id="gGauge"></div></div></div></div></fieldset>';

					    jQuery('#chartResult').html(chartHtml);
						drawChart(localLine, 'lLine');
					    drawGauge(localGauge, 'lGauge');
					    drawChart(globalLine, 'gLine');
					    drawGauge(globalGauge, 'gGauge');
					}
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
		animateDiv();
		setTimeout(function() {
		   jQuery('img.scope').hide();
		}, 5000);
		jQuery('img.scope').click(function(){
			jQuery(this).hide();
		});
		
		
		jQuery("#alertModel").hide();
		jQuery('#queryText').typeahead({source:[
		'ana maria braga',
		'baixaki',
		'caixa economica federal',
		'casa e video',
		'claro',
		'concursos',
		'detran',
		'esporte',
		'frases de amor',
		'funk',
		'globo',
		'gmail',
		'google',
		'hotmail',
		'ig',
		'jogos de meninas',
		'jogos online',
		'mario',
		'mercado livre',
		'msn',
		'naruto',
		'oi',
		'orkut',
		'panico',
		'previsao do tempo',
		'poquer',
		'receita federal',
		'record',
		'rio de janeiro',
		'terra',
		'tmp',
		'uol',
		'vivo',
		'yahoo',
		'youtube'
		],
		minLength:0
		});

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



    function makeNewPosition(){
	
	// Get viewport dimensions (remove the dimension of the div)
	var h = $(window).height() - 50;
	var w = $(window).width() - 50;
	
	var nh = Math.floor(Math.random() * h);
	var nw = Math.floor(Math.random() * w);
	
	return [nh,nw];    
	
    }

    function animateDiv(){
	var newq = makeNewPosition();
	var oldq = $('.scope').offset();
	var speed = calcSpeed([oldq.top, oldq.left], newq);
	
	$('.scope').animate({ top: newq[0], left: newq[1] }, speed, function(){
	  animateDiv();        
	});
	
    };

    function calcSpeed(prev, next) {
	
	var x = Math.abs(prev[1] - next[1]);
	var y = Math.abs(prev[0] - next[0]);
	
	var greatest = x > y ? x : y;
	
	var speedModifier = 0.1;

	var speed = Math.ceil(greatest/speedModifier);

	return speed;

    }

    </script>


</html>
