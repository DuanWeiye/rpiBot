// farchart farchart.js version 0.0.1, 28 August 2013 

// (c) 2013 by Jia ZhiGang. All Rights reserved. 
// Copyright (c) 2013 Jia ZhiGang (http://www.edinme.com)
//
// far chart is freely distributable under the terms of an GPL-style license.
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// For details, see the farchart web site: http://www.edinme.com
function check_vmlCapability() {
	if(document.namespaces['v']==null) {
		var e=["shape","shapetype","group","background","path","formulas","handles","fill","stroke","shadow","textbox","textpath","imagedata","line","polyline","curve","roundrect","oval","rect","arc","image"],s=document.createStyleSheet(); 
		for(var i=0; i<e.length; i++) {s.addRule("v\\:"+e[i],"behavior: url(#default#VML); display: inline-block;");} document.namespaces.add("v","urn:schemas-microsoft-com:vml");
	} 
}

function msieversion()
{
   var ua = window.navigator.userAgent;
   var msie = ua.indexOf ( "MSIE " );

   if ( msie > 0 ) 
   {
	   // If Internet Explorer, return version number
	   return parseInt (ua.substring (msie+5, ua.indexOf (".", msie )));	   
   }      
   else
   {
		// If another browser, return 0	   
		return 0;
	}                
}

var fishChart = {
	type: "null", // global flag, default
    width: 600,
    height: 400,
    series: [],
    unit: "kg",
    container: null,
	title: {
		text: "Far Chart",
		x: -100 // center offset
	},
    isIE: true,
    legend : {
    	enable : true
    },
    edge : {
    	width: 50, // for pie
    	height: 50, // for pie
		left: 50, // for other plot
		upper: 40,
		bottom: 40,
		right: 60
    },
	yAxis: {
		title: "Y Axis",
		vgap: 0,
		plotHeight: 0,
		min: 0,
		max: 0,
		tickSize: 10,
		padding: 5
	},
	xAxis: {
		categories: [],
		tickSize : 10,
		autoTick:true,
		xgap : 0,
		min:0,
		max:0,
		title:"X Axis"
	},
    animation: {
    	enable: true,
    	hh: 1,
		animCanvas:null,
		pctx: null
    },
    tooltip: {
    	enable: true,
    	tooltipCanvas : null,
    	ttContext: null,
    	index: -1
    },
    circle : { // for pie
    	cx: 0,
    	cy: 0,
    	radius: 0
    	
    },
    text : { // for pie
    	enable: false,
    	content:[]
    },
    
	point : { // for line and scatter plot
		enable : true,
		radius : 4
	},
    
    initSettings: function (config) {
    	this.type = config.type;
    	this.container = config.container;
        this.width = config.width;
        this.height = config.height;
        this.series = config.series;
        this.title.text = config.title;
        this.unit = config.unit;
        
        // tool-tip, animation, legend setting data
        if(config.tooltip != undefined) {
        	this.tooltip.enable = config.tooltip.enable;        	
        }
        if(config.animation != undefined) {
        	this.animation.enable = config.animation.enable;        	
        }
        if(config.legend != undefined) {
        	this.legend.enable = config.legend.enable;        	
        }
        if(config.text != undefined) {
        	this.text.enable = config.text.enable;
        }
        
        // edge setting data
        if(config.edge != undefined && config.edge.right != undefined) {
        	this.edge.right = config.edge.right;        	
        }
        if(config.edge != undefined && config.edge.left != undefined) {
        	this.edge.left = config.edge.left;        	
        }
        if(config.edge != undefined && config.edge.bottom != undefined) {
        	this.edge.bottom = config.edge.bottom;        	
        }
        if(config.edge != undefined && config.edge.upper != undefined) {
        	this.edge.upper = config.edge.upper;        	
        }
        
        // xAxis setting
        if(config.xAxis != undefined) {        	
        	this.xAxis.title= config.xAxis.title;
        	this.xAxis.tickSize = config.xAxis.tickSize;
        }
        
        if(config.xAxis != undefined && config.xAxis.categories != undefined) {
        	this.xAxis.categories = config.xAxis.categories;
        }
        
        // yAxis setting
        if(config.yAxis != undefined) {
        	this.yAxis.title = config.yAxis.title;
        	this.yAxis.tickSize = config.yAxis.tickSize;        	
        } 
        
        // decide whether render plot using HTML5 Canvas or VML
        if(msieversion() == 0) {
        	this.chartCanvas = document.createElement("canvas");
        	this.chartCanvas.id = "fc_canvas";
        	this.container.appendChild(this.chartCanvas);
        	this.chartCanvas.width = config.width;
        	this.chartCanvas.height = config.height;
        	this.isIE = false;
        } else {
        	check_vmlCapability();
        	this.isIE = true;
        }
    },
    
    render : function() {    	
    	var ctx = this.getCanvasContext();
    	this.renderBorder(ctx);
    	if(this.type === "pie") {
        	// initialization circle
        	this.circle.cx = this.width/2;
        	this.circle.cy = this.height/2;
        	this.circle.radius = Math.min(this.width/2, this.height/2) - Math.max(this.edge.width, this.edge.height);
        	if(this.circle.radius <= 0) {
        		console.log("Can not reader the chart, Circle is too small.");
        		return;
        	}
        	
        	// draw each arc according to data series 
        	var sum = 0;
        	var nums = this.series.length;
        	for(var i=0; i<nums; i++) {
        		sum += this.series[i].value;
        	}
        	
        	// draw title
        	this.customFillText(ctx, this.width/2 - this.edge.width, (this.isIE ? 10 : 30), this.circle.radius, 40, this.title.text, "white", "white", "black", '18pt Calibri');
        	
        	if(this.isIE) {
        		this.renderPieShadow(ctx);
        	}
        	
        	// render bar
        	var start = 0;
        	for(var i=0; i<nums; i++) {
        		var precent = this.series[i].value/sum;
        		this.renderPie(ctx, i, start, precent);
        		if(this.isIE) {
        			start += precent;
        		} else {
        			start += 2*Math.PI * precent;        			
        		}
        	}
        	
        	// post add blur shadow
        	if(!this.isIE) {
        		this.renderPieShadow(ctx);
        	}
        	
        	// render legend
        	this.renderPieLegend(ctx, sum);
        	
    	} 
    	// line plot
    	else if(this.type == "line")
    	{
    		
    		// draw title
    		this.customFillText(ctx, (this.width/2 + this.title.x), (this.isIE ? 2 : this.edge.upper/2), this.width/2, 0, this.title.text, "white", "white", "black", '18pt Calibri');
    		// draw XY Axis
    		this.renderXYAxis(ctx);
    		// draw lines
        	var nums = this.series.length;
        	for(var i=0; i<nums; i++) {
        		this.renderOneLineCurve(ctx, i);
        	}
        	
    		
    	}
    	
    	// render legend
    	this.renderLegend(ctx);
    	
    	// enable tool-tip
    	if(this.tooltip.enable && !this.isIE && !this.animation.enable) {
    		var parent = this;
    		this.chartCanvas.addEventListener('mousemove', function(event) {
    			var x = event.pageX;
    			var y = event.pageY;
    			var canvas = event.target;
    			var bbox = canvas.getBoundingClientRect();
    			var loc = { x: x - bbox.left * (canvas.width  / bbox.width),
    					y: y - bbox.top  * (canvas.height / bbox.height)};
    			parent.showTooltips(loc, ctx);
    		}, false);            	
    	}
    	
    	// enable animation
    	if(this.animation.enable) {
    		var parent = this;
    		if(this.isIE) {
    			ctx.insertBefore(this.animation.animCanvas);	
    		}
    		setTimeout(function() {parent.playAnimation(parent);}, 1000/20);   	    
    	}
    },
    
    renderOneLineCurve : function(ctx, index) {
    	var size = this.series[index].data.length;
    	var plotwidth = this.width - this.edge.left - this.edge.right;
		var deltax = plotwidth/size;
		var xpos = this.edge.left;
		
		if(this.isIE) {
			var vpolyLine = document.createElement("v:polyline");
			vpolyLine.strokecolor=this.series[index].color;
			vpolyLine.id = "vml-fline-" + index;
			vpolyLine.filled="false"; // default is true, will fill area of close path
			var points = "";
			for(var i=0; i<size; i++) {
				var value = this.series[index].data[i];
				var deltay = Math.floor((value - this.yAxis.min) * this.yAxis.ygap);
				
				// highlight the data point by circle.
				if(this.point.enable) {
					var hvoval = document.createElement("v:oval");
					hvoval.style.left = (xpos + Math.floor(deltax/2) - this.point.radius);
					hvoval.style.top = (this.height - this.edge.bottom - deltay - this.point.radius);
					hvoval.style.width = (this.point.radius * 2) + "px";
					hvoval.style.height = (this.point.radius * 2) + "px";
					hvoval.strokecolor = this.series[index].color;
					hvoval.fillcolor = this.series[index].color;
					hvoval.title = this.xAxis.categories[i] + " " + this.series[index].name + " : " +  + value; // tool-tip
					ctx.insertBefore(hvoval);
				}
				// - end it				
				
				points += (xpos + Math.floor(deltax/2)) + "," + (this.height - this.edge.bottom - deltay) + " ";		
				xpos += deltax;
			}
			vpolyLine.strokeweight="1px";
			vpolyLine.points=points;
			ctx.insertBefore(vpolyLine);
		} 
		else 
		{
			ctx.save();
			ctx.beginPath();
			ctx.lineWidth = 1;
			for(var i=0; i<size; i++) {
				var value = this.series[index].data[i];
				var deltay = Math.floor((value - this.yAxis.min) * this.yAxis.ygap);
				ctx.strokeStyle=this.series[index].color;
				if(i == 0) {
					ctx.moveTo(xpos + Math.floor(deltax/2), this.height - this.edge.bottom - deltay);
				} else {
					ctx.lineTo(xpos + Math.floor(deltax/2), this.height - this.edge.bottom - deltay);
				}
				
				xpos += deltax;
			}    	
			ctx.stroke();
			ctx.restore();
			
			// draw dot circle
			if(!this.point.enable)
				return;
			ctx.save();
			var xpos = this.edge.left;
			for(var i=0; i<size; i++) {
				var value = this.series[index].data[i];
				var deltay = Math.floor((value - this.yAxis.min) * this.yAxis.ygap);
				ctx.beginPath();
				ctx.arc(xpos + Math.floor(deltax/2), this.height - this.edge.bottom - deltay, this.point.radius, 0, 2*Math.PI, false);
				ctx.closePath();
				ctx.fillStyle=this.series[index].color;
				ctx.fill();
				xpos += deltax;
			}
			ctx.restore();
		}
    },
    
    renderPieLegend : function(ctx, sum) {
    	if(!this.legend.enable) return;
    	var nums = this.series.length;
    	ctx.font = '10pt Calibri';
    	var pos = (this.width/2 > (this.circle.radius+50)) ? 50 : (this.circle.cx - this.circle.radius);
    	for(var i=0; i<nums; i++) {
    		var x = this.series[i].value/sum;
    		x = Math.round (x*100) / 100;
    		var tipStr =  this.series[i].name + ": " + (x * 100).toFixed(0) + "%";
    		this.series[i].precent = tipStr;
    		this.renderRect(ctx, (pos - 40), (20*i+10), 10, 10, this.series[i].color, this.series[i].color, 0);
    		this.customFillText(ctx, (pos - 25), (this.isIE ? (20*i + 1) : (20*i+20)), 100, 0, tipStr, "white", "white", "black", "10pt Calibri");
    	} 
    },
    
    renderPieShadow  : function(ctx) {
    	if(this.isIE) {
        	var voval = document.createElement("v:oval");
        	voval.style.left=(this.circle.cx - this.circle.radius) + "px";
        	voval.style.top=(this.circle.cy - this.circle.radius) + "px";
        	voval.style.width=this.circle.radius*2 + "px";
        	voval.style.height=this.circle.radius*2 + "px";
        	voval.style.position="absolute";
        	voval.strokecolor="RGB(150,150,150)";
        	voval.strokeweight="4pt";
        	ctx.insertBefore(voval);
    	} else {
        	ctx.save();
        	ctx.shadowColor = "black";
        	ctx.shadowOffsetX = 0;
        	ctx.shadowOffsetY = 0;
        	ctx.shadowBlur = 10;
        	ctx.beginPath();
        	ctx.arc(this.circle.cx, this.circle.cy, this.circle.radius, 0, Math.PI * 2, false);
        	ctx.closePath();
        	ctx.lineWidth = 1;
        	ctx.strokeStyle = "RGBA(127,127,127,1)";
        	ctx.stroke();
        	ctx.restore();
    	}
    },
    
    renderPie : function(ctx, index, startp, precent) {
    	if(this.isIE) {
        	var start = Math.round (startp*100); 
        	var end = Math.round (precent*100) + start;
        	var part = document.createElement("v:shape");
        	part.style.width=Math.min(this.width, this.height) + "px";
        	part.style.height=Math.min(this.width, this.height) + "px";
        	part.style.position="absolute";
        	part.id = "vml-pie-part-" + index;
        	var path = "";
        	for (var i= start; i <= end; i++) {
    	      x = Math.floor((Math.cos(i*Math.PI/50)*this.circle.radius)+this.circle.cx);
    	      y = Math.floor((Math.sin(i*Math.PI/50)*this.circle.radius)+this.circle.cy);
    	      xs = x.toString();
    	      ys = y.toString();
    	      path +=  xs + ", " + ys + ", ";
        	}
        	part.strokecolor=this.series[index].color;
        	part.strokeweight="0pt";
        	part.fillcolor = this.series[index].color;
        	part.path= "m " + this.circle.cx + "," + this.circle.cy +" l " + path + this.circle.cx + "," + this.circle.cy + " x e";
        	ctx.insertBefore(part);
        	
        	// render text content
    		if(this.text.enable) {    		
    			var textLine = document.createElement("v:line");
    			var fx = Math.floor((Math.cos((start + (end - start)/2)*Math.PI/50)*this.circle.radius)+this.circle.cx);
    			var fy = Math.floor((Math.sin((start + (end - start)/2)*Math.PI/50)*this.circle.radius)+this.circle.cy);
    			var tx = (fx < this.circle.cx) ? (fx - this.edge.width) : (fx + this.edge.width);
    			textLine.from = fx + "," + fy;
    			textLine.to = tx + "," + fy;
    			ctx.insertBefore(textLine);
    			
    			var textPos = (fx < this.circle.cx) ? (fx - this.edge.width*2) : (fx + this.edge.width);    			
    			this.customFillText(ctx, (textPos), (fy-20), 100, 0, this.series[index].name + ": " + (precent * 100).toFixed(0) + "%", "white", "white", "black", "10pt Calibri");
    		}
    		
    		if(this.tooltip.enable) {
    			var _parent = this;
    			part.title = "Index: " + (index + 1) + "\n" + this.series[index].name + ": " + this.series[index].value + this.unit + "\n" + 
    						this.series[index].name + ": " + (precent * 100).toFixed(0) + "%" + "\n";
    			part.attachEvent('onmouseover', function (event) {
    				var srcElement = event.srcElement;
    				srcElement.strokecolor= "white";
    				srcElement.strokeweight="3pt";
    			});
    			part.attachEvent('onmouseout', function (event) { 
    				var srcElement = event.srcElement;
    				srcElement.strokecolor=_parent.series[index].color;
    				srcElement.strokeweight="0pt";
    			});
    		}
    		
    	} else {    		
    		var endAngle = startp + 2*Math.PI*precent;
    		ctx.save();
    		ctx.beginPath();
    		ctx.arc(this.circle.cx, this.circle.cy, this.circle.radius, startp, endAngle, false);
    		ctx.moveTo(this.circle.cx, this.circle.cy);
    		ctx.lineTo(this.circle.cx + this.circle.radius * Math.cos(startp), this.circle.cy + this.circle.radius * Math.sin(startp));
    		ctx.lineTo(this.circle.cx + this.circle.radius * Math.cos(endAngle), this.circle.cy + this.circle.radius * Math.sin(endAngle));
    		ctx.lineTo(this.circle.cx, this.circle.cy);
    		ctx.closePath();
    		ctx.fillStyle = this.series[index].color;
    		ctx.strokeStyle = this.series[index].color;
    		ctx.fill();
    		ctx.stroke();
    		ctx.restore();
    		
        	// render text content
        	if(this.text.enable) {    		
        		var halfEndAngle = startp + Math.PI*precent;
        		var hx = this.circle.cx + this.circle.radius * Math.cos(halfEndAngle);
        		var hy = this.circle.cy + this.circle.radius * Math.sin(halfEndAngle);
        		ctx.beginPath();
        		ctx.moveTo(hx, hy);
        		var linePos = (hx < this.circle.cx) ? (hx - this.edge.width) : (hx + this.edge.width);
        		ctx.lineTo(linePos, hy);
        		ctx.closePath();
        		ctx.strokeStyle="black";
        		ctx.stroke();
        		var textPos = (hx < this.circle.cx) ? (hx - this.edge.width*2) : (hx + this.edge.width);
        		precent = Math.round (precent*100) / 100;
        		var size = this.text.content.length;
        		var tipStr = (size > index) ? this.text.content[index] : this.series[index].name + ": " + (precent * 100).toFixed(0) + "%";
        		ctx.font = '10pt Calibri';
        		ctx.fillStyle="black";
        		ctx.fillText(tipStr, textPos, hy);
        	}
    	}
    },
    
    showLineTooltips : function(loc, ctx) {
    	if(!this.tooltip.enable) {
    		return;
    	}
    	var size = this.series[0].data.length;
    	var plotwidth = this.width - this.edge.left - this.edge.right;
		var deltax = (plotwidth/ size);
    	var xunit = ((loc.x - this.edge.left)/deltax); //: */loc.x - this.edge.left;
    	var deltay = this.height - this.edge.bottom - loc.y;
    	if(xunit > 0 && deltay > 0) {
    		var value = deltay/this.yAxis.ygap + this.yAxis.min;
    		var xindex = Math.floor(xunit);
    		var xpoint = xunit - xindex;
    		if(xpoint < 0.55 && xpoint >0.45) {
    			var num = this.series.length;
    			var distance = [];
    			for(var i=0; i<num; i++) {
    				var dis = this.series[i].data[xindex] - value;
    				distance[i] = Math.abs(dis);
    			}
    			var min = distance[0];
    			var yindex = 0;
    			for(var i=0; i<num; i++) {
    				if(min > distance[i]) {
    					min = distance[i];
    					yindex = i;
    				}
    			}
    			
    			if(this.series[yindex].data[xindex] > Math.floor(value - 3) && this.series[yindex].data[xindex] < Math.floor(value + 3) ) {
    				this.renderLineTooltips(ctx, yindex, xindex, loc);
    			} 
    			// clear tool tip
    			else {
    				this.clearTooltips(ctx);
    			}
    		} 
    		else {
    			this.clearTooltips(ctx);
    		}
    	} 
    	else {
    		this.clearTooltips(ctx);
    	}
    },
    
    renderLineTooltips : function(ctx, yindex, xindex, loc) {
		// show tool tip
		this.clearTooltips(ctx);
		if(this.tooltip.tooltipCanvas == null) {
			this.tooltip.tooltipCanvas = document.createElement("canvas");
			this.tooltip.ttContext = this.tooltip.tooltipCanvas.getContext("2d");
    		this.tooltip.tooltipCanvas.width = 150;
    		this.tooltip.tooltipCanvas.height = 100;
		}
		var m_context = this.tooltip.ttContext;
		m_context.save();
		m_context.clearRect(0, 0, this.tooltip.tooltipCanvas.width, this.tooltip.tooltipCanvas.height);
		m_context.lineWidth = 2;
		m_context.strokeStyle = this.series[yindex].color;
		m_context.fillStyle="RGBA(255,255,255,0.7)";
		this.getRoundRect(m_context, 2,2,this.tooltip.tooltipCanvas.width-4, this.tooltip.tooltipCanvas.height-4, 5, true, true);
		m_context.font="14px Arial";
		m_context.fillStyle="RGBA(0,0,0,1)";
		if(this.type == "line") {
			m_context.fillText((this.xAxis.title + ": " + this.xAxis.categories[xindex]), 5, 20);
			m_context.fillText(this.series[yindex].name + ": " + this.series[yindex].data[xindex], 5, 40);			
		} else {
			m_context.fillText(this.series[yindex].name, 5, 20);
			m_context.fillText(this.xAxis.title + ":" + this.series[yindex].data[xindex][0], 5, 40);
			m_context.fillText(this.yAxis.title + ":" + this.series[yindex].data[xindex][1], 5, 60);
		}
		m_context.restore();
		
		// make tool-tip rectangle is always visible 
		if((loc.x + this.tooltip.tooltipCanvas.width)> this.width) {
			loc.x = loc.x - this.tooltip.tooltipCanvas.width;
		}
		if((loc.y - this.tooltip.tooltipCanvas.height) <= 0) {
			loc.y = loc.y + this.tooltip.tooltipCanvas.height;
		}
		ctx.drawImage(this.tooltip.tooltipCanvas, 0, 0, this.tooltip.tooltipCanvas.width, this.tooltip.tooltipCanvas.height, 
				loc.x, loc.y-this.tooltip.tooltipCanvas.height, this.tooltip.tooltipCanvas.width, this.tooltip.tooltipCanvas.height);
    },
    
    showPieTooltips : function(loc, ctx) {
    	if(!this.tooltip.enable) {
    		return;
    	}
    	var dx = loc.x - this.width/2;
    	var dy = loc.y - this.height/2;
    	var dis = Math.floor(Math.sqrt(dx * dx + dy * dy));
    	if(dis <= this.circle.radius) {
    		// draw tool tip text
    		var angle = Math.atan2(dy,dx);
    		if(angle <= 0) {
    			// if[-Math.PI, 0], make it[Math.PI, 2*Math.PI]
    			angle = angle + 2*Math.PI;
    		}
    		
        	var sum = 0;
        	var nums = this.series.length;
        	for(var s=0; s<nums; s++) {
        		sum += this.series[s].value;
        	}
        	
        	var deltaArc = 0;
        	var index = 0;
        	for(var i=0; i<nums; i++) {
        		var precent = this.series[i].value/sum;
        		deltaArc += 2*Math.PI * precent;
        		if(angle<=deltaArc) {
        			index = i;
        			break;
        		}
        	}
    		if(this.tooltip.tooltipCanvas == null) {
    			this.tooltip.tooltipCanvas = document.createElement("canvas");
    			this.tooltip.ttContext = this.tooltip.tooltipCanvas.getContext("2d");
        		this.tooltip.tooltipCanvas.width = 150;
        		this.tooltip.tooltipCanvas.height = 100;
    		} 
    		
    		// only draw once
    		// if(index == this.tooltips.index){
    		// 	return;
    		// }
    		this.clearTooltips(ctx);
    		
    		this.tooltip.index = index;
    		var m_context = this.tooltip.ttContext;
    		m_context.save();
    		m_context.clearRect(0, 0, this.tooltip.tooltipCanvas.width, this.tooltip.tooltipCanvas.height);
    		m_context.lineWidth = 2;
    		m_context.strokeStyle = this.series[index].color;
    		m_context.fillStyle="RGBA(255,255,255,0.7)";
    		// m_context.strokeRect(2, 2, this.tooltips.tooltipCanvas.width-4, this.tooltips.tooltipCanvas.height-4);
    		// m_context.fillRect(2,2,this.tooltips.tooltipCanvas.width-4, this.tooltips.tooltipCanvas.height-4);
    		this.getRoundRect(m_context, 2,2,this.tooltip.tooltipCanvas.width-4, this.tooltip.tooltipCanvas.height-4, 5, true, true);
			m_context.font="14px Arial";
			m_context.fillStyle="RGBA(0,0,0,1)";
			m_context.fillText("Index: " + (index + 1), 5, 20);
			m_context.fillText(this.series[index].name + ": " + this.series[index].value + this.unit, 5, 40);
			m_context.fillText(this.series[index].precent, 5, 60);
			m_context.restore();
			
			// make tool-tip rectangle is always visible 
			if((loc.x + this.tooltip.tooltipCanvas.width)> this.width) {
				loc.x = loc.x - this.tooltip.tooltipCanvas.width;
			}
			if((loc.y - this.tooltip.tooltipCanvas.height) <= 0) {
				loc.y = loc.y + this.tooltip.tooltipCanvas.height;
			}
			ctx.drawImage(this.tooltip.tooltipCanvas, 0, 0, this.tooltip.tooltipCanvas.width, this.tooltip.tooltipCanvas.height, 
					loc.x, loc.y-this.tooltip.tooltipCanvas.height, this.tooltip.tooltipCanvas.width, this.tooltip.tooltipCanvas.height);	
    	} else {
    		this.tooltip.index = -1;
    		this.clearTooltips(ctx);
    	}
    },
    
    redrawPie : function(ctx) {
    	if(this.animation.enable) {
    		ctx.clearRect(0,0,this.width, this.height);
    		this.renderBorder(ctx);
    		ctx.drawImage(this.animation.animCanvas, 0, 0, this.width, this.height, 0, 0, this.width, this.height); 
    	} else {
        	// draw each arc according to data series 
        	var sum = 0;
        	var nums = this.series.length;
        	for(var i=0; i<nums; i++) {
        		sum += this.series[i].value;
        	}
        	
        	// draw title
        	this.customFillText(ctx, this.width/2 - this.edge.width, (this.isIE ? 10 : 30), this.circle.radius, 40, this.title, "white", "white", "black", '18pt Calibri');
        	
        	if(this.isIE) {
        		this.renderPieShadow(ctx);
        	}
        	
        	// render bar
        	var start = 0;
        	for(var i=0; i<nums; i++) {
        		var precent = this.series[i].value/sum;
        		this.renderPie(ctx, i, start, precent);
        		if(this.isIE) {
        			start += precent;
        		} else {
        			start += 2*Math.PI * precent;        			
        		}
        	}
        	
        	// post add blur shadow
        	if(!this.isIE) {
        		this.renderPieShadow(ctx);
        	}
        	
        	// render legend
        	this.renderPieLegend(ctx, sum);
    	}
    },
    
    renderXYAxis : function(ctx) {
    	this.drawXYTitles(ctx);
    	if(this.type == "line") {
        	// draw ticks and markers
        	var nums = this.series.length;
        	if(nums == 0) return;
    		
    		var min = 0;
    		var max = 0;
    		min = this.series[0].data[0];
    		max = this.series[0].data[0];
    		for(var s = 0; s < nums; s++) {
    			var size = this.series[s].data.length;
    			for(var i=0; i<size; i++) {
    				min = Math.min(min, this.series[s].data[i]);
    				max = Math.max(max, this.series[s].data[i]);
    			}
    		}    		
        	
        	// calculate the text gap in Y Axis
        	var delta = max - min;
        	this.yAxis.padding = delta/10;
        	this.yAxis.min = min - this.yAxis.padding;
        	this.yAxis.max = max + this.yAxis.padding;
        	var sum = this.yAxis.max - this.yAxis.min;
        	var plotHeight = this.height - this.edge.upper - this.edge.bottom;
        	var ygap = plotHeight/sum;
        	this.yAxis.plotheight = plotHeight;
        	this.yAxis.ygap = ygap;
        	var index = Math.floor(this.yAxis.min);
        	var first = true;
    		// draw Y Axis and dash line
        	for(var ypos=0; ypos<(plotHeight + this.edge.upper/2); ypos+=ygap*this.yAxis.tickSize)
        	{
        		this.customFillText(ctx, this.isIE ? (this.edge.left/3) : this.edge.left/2, this.isIE ? (this.height - this.edge.bottom - Math.floor(ypos)-15) : this.height - this.edge.bottom - Math.floor(ypos), 
        				40, 0, "" + index, "white", "white", "black", "10px Arial");
        		index = index + this.yAxis.tickSize;
        		
        		// draw dash line
        		if(!first) {
        			this.dashedLineTo(ctx, this.edge.left, this.height - this.edge.bottom - Math.floor(ypos), this.width - this.edge.right, this.height - this.edge.bottom - Math.floor(ypos), 5);        				       			
        		}
        		first = false;
        	}
        	
        	// draw X Axis sub title
    		var csize = this.xAxis.categories.length;
    		var plotwidth = this.width - this.edge.left - this.edge.right;
    		delta = plotwidth/ csize;
    		var xpos = this.edge.left;
    		for(var x=0; x<csize; x++) {
    			this.customFillText(ctx, (xpos + delta/2), this.isIE ? (this.height - (this.edge.bottom*0.75) - 15):this.height - (this.edge.bottom*0.75), 
    					50, 0, this.xAxis.categories[x], "white", "white", "black", "10px Arial");
    			xpos += delta;
    		}
        	
    	} else if(this.type == "scatter") {
    		
    	} else if(this.type == "column") {
    		
    	} else if(this.type == "bar") {
    		
    	}
    },
    
    drawXYTitles : function(ctx) {
    	if(this.isIE) {
        	// draw X Axis and title
        	var xLine = document.createElement("v:line");
        	xLine.from = this.edge.left + "," + (this.height-this.edge.bottom);
        	xLine.to = (this.width - this.edge.right) + "," + (this.height-this.edge.bottom);
        	ctx.insertBefore(xLine);
        	this.customFillText(ctx, this.width/2, this.height-this.edge.bottom/2, (this.width/2 - this.edge.right), 0, this.xAxis.title, "white", "white", "black", "12px Arial");

        	// draw Y title, VML rotate text is very trick thing,
    		// the center point is itself half of width and height, very bad way
    		var yTitleRect = document.createElement("v:rect");
    		yTitleRect.style.left= Math.floor(-this.height/2 + 10) + "px";
    		yTitleRect.style.top= (this.height - this.edge.bottom) + "px";
    		yTitleRect.style.width=Math.floor(this.height) + "px";
    		yTitleRect.style.height="0px";
    		yTitleRect.fillcolor = "white";
    		yTitleRect.strokecolor="white";
    		yTitleRect.strokeweight="0pt";
    		
    		var ytitleTextBox = document.createElement("v:textbox");
    		ytitleTextBox.style.font = "12px Arial";
    		ytitleTextBox.innerText = this.yAxis.title;
    		yTitleRect.style.rotation = "90"; // only make the text vertical, bad!!!
    		yTitleRect.insertBefore(ytitleTextBox);		
    		ctx.insertBefore(yTitleRect);
    		
    	} else {
    		// draw X Axis and title
    		ctx.save();
    		this.onePixelLineTo(ctx, this.edge.left, this.height-this.edge.bottom, this.width - this.edge.right, this.height-this.edge.bottom, "white", false);  	
    		ctx.restore();
    		ctx.font="12px Arial";
    		ctx.fillText(this.xAxis.title, this.width/2, this.height-this.edge.bottom/4);
    		
    		// draw Y title
    		ctx.save();
    		ctx.font="12px Arial";
    		ctx.translate(this.width/2, this.height/2);
    		ctx.rotate(-Math.PI/2);
    		ctx.fillText(this.yAxis.title, 20, -(this.width/2 - 20));
    		ctx.restore();    		
    	}
    },
    
    renderLegend : function(ctx) {
    	if(!this.legend.enable || this.type == "pie") {
    		return;
    	}
    	// TODO:zhigang, legend here
    },
    
    renderBorder : function(ctx) {
    	this.renderRect(ctx, 0, 0, this.width, this.height, "white", "black", 1);
    },
    
    playAnimation : function(parent) {
    	
    	if(parent.animation.hh < parent.height) {
    		if(parent.isIE) {
    			var node = document.getElementById("fishchart_mask_rect");
    			node.style.top= this.animation.hh + "px";
    			node.style.height= (this.height - this.animation.hh) + "px";
    			parent.animation.hh = parent.animation.hh + 10;
    			setTimeout(function() {parent.playAnimation(parent);}, 1000/20);
    		} else {
    			parent.animation.pctx.save();
    			parent.animation.pctx.globalAlpha=0.5;
    			parent.animation.pctx.clearRect(0,0,parent.width, parent.height);
    			parent.renderBorder(parent.animation.pctx);
    			parent.animation.pctx.drawImage(parent.animation.animCanvas, 0, 0, parent.width, this.animation.hh, 0, 0, parent.width, this.animation.hh);
    			parent.animation.hh = parent.animation.hh + 10;
    			parent.animation.pctx.restore();    	    		
    			setTimeout(function() {parent.playAnimation(parent);}, 1000/20);     			
    		}
    	} else {
    		// remove xxxxx or change the alpha value
    		if(parent.isIE) {    			
    	    	var node = document.getElementById("fishchart_mask_rect");
    	    	if(node != null) { 	    		
    	    		parent.animation.pctx.removeChild(node);    		
    	    	}
    	    	
    		} else {
    			parent.animation.pctx.clearRect(0,0,parent.width, parent.height);
    			parent.renderBorder(parent.animation.pctx);
    			parent.animation.pctx.drawImage(parent.animation.animCanvas, 0, 0, parent.width, parent.height, 0, 0, parent.width, parent.height);   			
    		}
    		
    		// enable tool-tip functionality
        	if(!this.isIE && parent.animation.enable && parent.tooltip.enable) {
        		parent.chartCanvas.addEventListener('mousemove', function(event) {
    	    		var x = event.pageX;
    	    		var y = event.pageY;
    	    		var canvas = event.target;
    	    		var bbox = canvas.getBoundingClientRect();
    	    		var loc = { x: x - bbox.left * (canvas.width  / bbox.width),
    	    				y: y - bbox.top  * (canvas.height / bbox.height)};
    	    		console.log("ddddddddddddddddddddd");
    	    		parent.showTooltips(loc, (parent.animation.enable ? parent.animation.pctx : ctx));
    	        }, false);
        	}
    	}
    },
    
    clearTooltips : function(ctx) {
		if(this.animation.enable) {
			ctx.clearRect(0,0,this.width, this.height);
			this.renderBorder(ctx);
			ctx.drawImage(this.animation.animCanvas, 0, 0, this.width, this.height, 0, 0, this.width, this.height); 			
		} else {			
			this.redrawPlot(ctx);
		}
    },
    
    redrawPlot : function(ctx) {
    	if(this.type === "pie") {
    		this.redrawPie(ctx);
    	} else if(this.type == "line") {
    		ctx.clearRect(0,0,this.width, this.height);
    		// draw title
    		this.customFillText(ctx, (this.width/2 + this.title.x), (this.isIE ? 2 : this.edge.upper/2), this.width/2, 0, this.title.text, "white", "white", "black", '18pt Calibri');
    		// draw XY Axis
    		this.renderXYAxis(ctx);
    		// draw lines
        	var nums = this.series.length;
        	for(var i=0; i<nums; i++) {
        		this.renderOneLineCurve(ctx, i);
        	}
    	}
    },
    
    showTooltips : function(loc, ctx) {
    	if(this.type === "pie") {
    		this.showPieTooltips(loc, ctx);
    	} else if(this.type === "line") {
    		this.showLineTooltips(loc, ctx);
    	}
    },
    
    getCanvasContext : function() {
    	var ctx = null;
    	if(this.isIE) {
    		// create group
    		ctx = document.createElement("v:group");
    		var min_size = Math.min(this.width, this.height);
    		ctx.style.width = min_size;
    		ctx.style.height = min_size;
    		
    		// default the group will be center inside the DIV element
    		ctx.coordsize = min_size + ' ' + min_size;
    		this.container.insertBefore(ctx);
    		if(this.animation.enable) {
    			this.animation.animCanvas = document.createElement("v:rect");
    			this.animation.animCanvas.id = "fishchart_mask_rect";
    			this.animation.animCanvas.style.left= "0px";
    			this.animation.animCanvas.style.top= "0px";
    			this.animation.animCanvas.style.width= this.width + "px";
    			this.animation.animCanvas.style.height= this.height + "px";
    			this.animation.animCanvas.fillcolor = "white";
    			this.animation.animCanvas.strokecolor = "white";
    			this.animation.animCanvas.strokeweight= "0pt";
    			var ttfill = document.createElement("v:fill");
    			ttfill.opacity = "100%";
    			this.animation.animCanvas.insertBefore(ttfill);	
    			this.animation.pctx = ctx;
    		}
    	} else {
        	if(this.animation.enable) {
        		this.animation.animCanvas = document.createElement("canvas");
            	this.animation.animCanvas.width = this.width;
            	this.animation.animCanvas.height = this.height;
            	this.animation.pctx = this.chartCanvas.getContext("2d"); // parent ctx
            	ctx = this.animation.animCanvas.getContext("2d");
        	} else {
        		ctx = this.chartCanvas.getContext("2d");
        	}
    	}
    	return ctx;
    },
    
    getRoundRect : function(ctx, x, y, width, height, radius, fill, stroke) {
		if (typeof stroke == "undefined") {
			stroke = true;
		}
		if (typeof radius === "undefined") {
			radius = 5;
		}
		ctx.beginPath();
		ctx.moveTo(x + radius, y);
		ctx.lineTo(x + width - radius, y);
		ctx.quadraticCurveTo(x + width, y, x + width, y + radius);
		ctx.lineTo(x + width, y + height - radius);
		ctx.quadraticCurveTo(x + width, y + height, x + width - radius, y+ height);
		ctx.lineTo(x + radius, y + height);
		ctx.quadraticCurveTo(x, y + height, x, y + height - radius);
		ctx.lineTo(x, y + radius);
		ctx.quadraticCurveTo(x, y, x + radius, y);
		ctx.closePath();
		if (stroke) {
			ctx.stroke();
		}
		if (fill) {
			ctx.fill();
		}
    },
    
    renderRect : function (ctx, x, y, width, height, fillColor, strokeColor, lineWidth) 
    {
    	if(this.isIE) {
    		var vrect = document.createElement("v:rect");
    		vrect.style.left= x + "px";
    		vrect.style.top= y + "px";
    		vrect.style.width= width + "px";
    		vrect.style.height= height + "px";
    		vrect.fillcolor = fillColor;
    		vrect.strokecolor=strokeColor;
    		vrect.strokeweight= lineWidth + "pt";
    		ctx.insertBefore(vrect);
    	} else {
    		ctx.save();
    		ctx.fillStyle= fillColor;
    		ctx.strokeStyle= strokeColor;
    		ctx.fillRect(x, y, width, height);
    		ctx.strokeRect(x, y, width, height);
    		ctx.restore();
    	}
    },
    
    customFillText : function (ctx, x, y, width, height, textContent, fillColor, strokeColor, fontColor, fontSize)
    {
    	if(this.isIE) {
    		var textRect = document.createElement("v:rect");
    		textRect.style.left=(x) + "px";
    		textRect.style.top=(y) + "px";
    		textRect.style.width=width + "px";
    		textRect.style.height=height + "px";
    		textRect.fillcolor = fillColor;
    		textRect.strokecolor = strokeColor;
    		textRect.strokeweight="0pt";
    		var textBox = document.createElement("v:textbox");
    		textBox.style.font = fontSize;// "10pt Calibri";
    		textBox.innerText = textContent;
    		
			// make stroke and fill attribute transparent is 100%
			var ttfill = document.createElement("v:fill");
			ttfill.opacity = "0%";
			var ttstroke = document.createElement("v:stroke");
			ttstroke.opacity = "0%";
			
			// append to group element
			textRect.insertBefore(ttstroke);
			textRect.insertBefore(ttfill);			
    		textRect.insertBefore(textBox);
    		ctx.insertBefore(textRect);    		
    	} else {
    		ctx.save();
    		ctx.font = fontSize;//'10pt Calibri';
    		ctx.fillStyle=fontColor;
    		ctx.fillText(textContent, x, y);
    		ctx.restore();
    	}
    },
    
    onePixelLineTo : function(ctx, fromX, fromY, toX, toY, backgroundColor, vertical) {
    	var currentStrokeStyle = this.strokeStyle;
    	ctx.beginPath();
    	ctx.moveTo(fromX, fromY);
    	ctx.lineTo(toX, toY);
    	ctx.closePath();
    	ctx.lineWidth=2;
    	ctx.stroke();
    	ctx.beginPath();
    	if(vertical) {
    		ctx.moveTo(fromX+1, fromY);
    		ctx.lineTo(toX+1, toY);
    	} else {
    		ctx.moveTo(fromX, fromY+1);
    		ctx.lineTo(toX, toY+1);
    	}
    	ctx.closePath();
    	ctx.lineWidth=2;
    	ctx.strokeStyle=backgroundColor;
    	ctx.stroke();
    	ctx.strokeStyle = currentStrokeStyle;
    },
    
    dashedLineTo : function (ctx, fromX, fromY, toX, toY, pattern) {
		if(this.isIE) {
			var vdashline = document.createElement("v:line");
			var vdashlineStroke = document.createElement("v:stroke");
			vdashline.from = fromX + "," + fromY;
			vdashline.to = toX + "," + toY;
			vdashlineStroke.dashstyle="dash";
			vdashline.insertBefore(vdashlineStroke);
			ctx.insertBefore(vdashline);        				
		} else {
	    	ctx.save();
	    	ctx.lineWidth = 1;
	    	ctx.translate(0.5,0.5);
			// default interval distance -> 5px
	    	if (typeof pattern === "undefined") {
	    		pattern = 5;
	    	}
	
	    	// calculate the delta x and delta y
	    	var dx = (toX - fromX);
	    	var dy = (toY - fromY);
	    	var distance = Math.floor(Math.sqrt(dx*dx + dy*dy));
	    	var dashlineInteveral = (pattern <= 0) ? distance : (distance/pattern);
	    	var deltay = Math.floor((dy/distance) * pattern);
	    	var deltax = Math.floor((dx/distance) * pattern);
	    	
	    	// draw dash line
	    	ctx.beginPath();
	    	for(var dl=0; dl<dashlineInteveral; dl++) {
	    		if(dl%2) {
	    			ctx.lineTo(fromX + dl*deltax, fromY + dl*deltay);
	    		} else {    				
	    			ctx.moveTo(fromX + dl*deltax, fromY + dl*deltay);    				
	    		}    			
	    	}
	    	ctx.stroke();
	    	ctx.restore();
		}
    }
    
};