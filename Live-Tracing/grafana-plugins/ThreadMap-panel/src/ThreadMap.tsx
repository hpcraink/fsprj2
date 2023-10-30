import React, { useEffect } from 'react';
import { PanelProps } from '@grafana/data';
import { PanelOptions } from 'options';
import _ from 'lodash';
import { CustomScrollbar } from '@grafana/ui'; //npnmistall @grafana/ui
import * as d3 from 'd3';

interface ThreadMapPanelProps extends PanelProps<PanelOptions> {}

export const ThreadMap: React.FC<ThreadMapPanelProps> = ({ options, data, width, height}) => {
  useEffect(() => {
    var ProcessColour = new Array     
    var ThreadColour = new Array      
    var Colours = new Array           //Possible Colour Amount
    var FilesystemValue = new Array
    var DataLength = 0                //Datalength libiotrace w/o filesystem
    var MapData: any = []
    var SepparatorData: any = []

    //Amount Threads (written)
    if(DataLength == 0) {
      for (let i = 0; i < data.series.length; i++) { 
        if ((data.series[i].fields[1] !== undefined) && (data.series[i].refId === "Written Bytes" || data.series[i].refId === "Read Bytes")) {
          DataLength++
        }
      }
    }

    //Coloring based on amonut written bytes
    Colours = ColourSteps(DataLength)
    const ColAssig = ColourAssignment(Colours, DataLength)
    ThreadColour = ColAssig[0]?.[0]
    ProcessColour = ColAssig[1]
    FilesystemValue = FilesystemAssignment()

    //Assign min/max values to default Options
    let minVal: number, maxVal: number
    if (options.UseMinMaxBoolean === false) {
      options.ThreadMapColor.min = minVal = ColAssig[0]?.[1]
      options.ThreadMapColor.max = maxVal = ColAssig[0]?.[2]
    }
    else  {
      minVal = options.ThreadMapColor.min
      maxVal = options.ThreadMapColor.max
    }
    let minValLength = (minVal.toString().length) * 11

    //Generate Data for process, threads and filesystem
    let k = 0
    for (let i = 0;i < DataLength; i++) {
      if(ProcessColour[i] !== undefined) 
      {
        BuildPanelProcess(i, k, ProcessColour[i], ColAssig[2][i])
        k++        
      }
      BuildPanelThread(i, k, ThreadColour[i])
      k++
    }

    //Call Filter
    if(options.UseFileSystemFinder) {
      FileSystemFinder();
    }

    drawThreadMap()

    //functions
    function ColourSteps(lDdatalength: any)
    {
      let Colour = new Array
      const step = 255 / ((lDdatalength-1)*0.5)
      Colour[0] = '#00ff00'

      for (let i = 1; i < (lDdatalength); i++) {
        if(i < (lDdatalength/2)) { //green #00FF00 to yellow #FFFF00
          if ((i*step) < 16) {
            Colour[i] = '#0' + (i*step).toString(16).substring(0,1) + 'ff00'
          }
          else {
          Colour[i] = '#' + (i*step).toString(16).substring(0,2) + 'ff00'
          }
        } 
        else { //yellow #FFFF00 to red #FF0000 
          if (((i-lDdatalength/2)*step) >= 239) {
            Colour[i] = '#ff0' + (255-((i-lDdatalength/2)*step)).toString(16).substring(0,1) + '00'
          }
          else {
          Colour[(i)] = '#ff' + (255-((i-lDdatalength/2)*step)).toString(16).substring(0,2) + '00'
          }
        }
      }
      return(Colour)
    }

    function ColourAssignment(Colour: any, lDatalength: any) {
      let ctrTpP = 0
      var ThreadHeatValue: any[] = []
      var ProcessHeatValue: number[] = []
      ProcessHeatValue[0] = 0
      var ReturnColourThreads = new Array
      var ReturnColourProcess = new Array

      for (let i = 0; i < lDatalength; i++) {
        ThreadHeatValue[i] = data.series[i].fields[1].values.get(0)
      }

      for (let ictr = 0; ictr < (lDatalength-ctrTpP);) {    
        if (!(data.series[ictr].fields[1].labels?.processid === data.series[ictr+ctrTpP].fields[1].labels?.processid)) {
          //Next Process with different PID
          ictr = ictr + ctrTpP
          //Resert ctr
          ctrTpP=1
          ProcessHeatValue[ictr] = ThreadHeatValue[ictr]
        }
        else {
          ProcessHeatValue[ictr] = ProcessHeatValue[ictr] + ThreadHeatValue[ictr+ctrTpP]
          ctrTpP++
        }
      }
      ReturnColourThreads = Assignment(ThreadHeatValue)
      ReturnColourProcess = Assignment(ProcessHeatValue)

      function Assignment(HeatValue: any)
      {
        //remove undefined Values for min/max
        const MathValue = HeatValue.filter(function(element: any)
        {
          return element !== undefined
        });
        let min = Math.min(...MathValue)
        let max = Math.max(...MathValue)
        let test = 0
        var ReturnColour = new Array()
        let j = 0
        //min/max Values from data
        if(options.UseMinMaxBoolean === false){
          for (; HeatValue[j] >= (min+test*((max-min)/DataLength)) && (j < HeatValue.length);) { 
            if (HeatValue[j] <= (min+(test+1)*((max-min)/DataLength))){
              ReturnColour[j] = Colour[test]
              j++
              test = 0
              //Skip for PID
              while (HeatValue[j] === undefined && j < HeatValue.length) {
                j++
              }
            }
            else{
              test++
            }
          }
        }
        //specified min/max Values
        else{
          for (; ((HeatValue[j] >= (options.ThreadMapColor.min+test*((options.ThreadMapColor.max-options.ThreadMapColor.min)/DataLength))) || HeatValue[j] < options.ThreadMapColor.min) && (j < HeatValue.length);) { 
            if ((HeatValue[j] <= (options.ThreadMapColor.min+(test+1)*((options.ThreadMapColor.max-options.ThreadMapColor.min)/DataLength))) || (HeatValue[j] >= options.ThreadMapColor.max)){
              if (HeatValue[j] >= options.ThreadMapColor.max) {
                ReturnColour[j] = Colour[(Colour.length-1)]
              }
              else {
                ReturnColour[j] = Colour[test]  
              }
              j++
              test = 0
              //Skip for PID
              while (HeatValue[j] === undefined && j < HeatValue.length) {
                j++
              }
            }
            else{
              test++
            }
          }
        }
        return[ReturnColour, min, max]
      }
      //Return Min/Max only for Threads
      return[ReturnColourThreads, ReturnColourProcess[0], ProcessHeatValue]
    }

    function FilesystemAssignment() {
      //Transfer traced_filename to local var
      var lTracedFilename = new Array
      let lOutputFs: any[][] = [[]];
      for (let i = 0; i < data.series.length-DataLength; i++) {
        lTracedFilename[i] = data.series[i+DataLength].fields[1].values
      }
      
      //convert filename to filesystem
      for (let i = 0; i < lTracedFilename.length; i++) {
        const lFileSystem: string[] = [];
        for (let j = 0; j < lTracedFilename[i].buffer.length; j++) {
          const pathSegments = lTracedFilename[i].buffer[j].split("/");
          if (pathSegments[1] !== undefined) {
            if(!lFileSystem.includes(pathSegments[1])) {
              lFileSystem.push(pathSegments[1])
            }
          }
          else {
            if (!lFileSystem.includes(pathSegments[0])) {
            lFileSystem.push(pathSegments[0])
            }
          }
        }
        lOutputFs.push(lFileSystem)
      }
      //delete first Element
      lOutputFs.shift();
      return(lOutputFs)
    }

    function BuildPanelProcess(i: number, k: number, Colour: any, writtenBytes: any) {
      //Affiliated Threads
      let lThreads: any[] = []
      let j = i
      while(data.series[i].fields[1].labels?.processid === data.series[j].fields[1].labels?.processid) {
          lThreads.push(data.series[j].fields[1].labels?.thread)
          j++
      }
      //Add Process
      MapData.push(
        {
          index: k,
          prefix: 'ProzessID: ',
          name: data.series[i].fields[1].labels?.processid,
          wrbytes: 'Written & read Bytes: ' + writtenBytes,
          affiliatedPrefix: 'Threads:',
          affiliated: lThreads,
          cx:(15+20*i),
          cy: 60,
          r: 8,
          colour: Colour,
          class: "Process"
        }
      )
      //Add Separator
      SepparatorData.push(
        {
          x: (20*i+3),
          y: 40,
          width: 2,
          height: height - 175,
          colour: '#353535'
        }
      )
    }

    function BuildPanelThread(i: number, k: number, Colour: any) {
      MapData.push(
        {
          index: k,
          prefix: 'ThreadID: ',
          name: data.series[i].fields[1].labels?.thread,
          wrbytes: 'Written & read Bytes: ' + data.series[i].fields[1].values.get(0),
          affiliatedPrefix: 'Filesystems:',
          affiliated: FilesystemValue[i],
          cx:(15+20*i),
          cy: 140,
          r: 6,
          colour: Colour,
          class: "Thread",
          stroke: ''
        }
      )
    }
    
    function FileSystemFinder() {
      for (let i = 0; i < MapData.length; i++) {
        if (MapData[i].affiliated.includes(options.FileSystemFinder)) {
          MapData[i].stroke = 'pink';
        }       
      }
    }

    function drawThreadMap() {
      //Redraw if new Min/Max Vals are specified
      if ((options.ThreadMapColor.min !== ColAssig[0]?.[1]) || (options.ThreadMapColor.max !== ColAssig[0]?.[2])) {
        d3.select('#ThreadMapMain').selectAll('*').remove();
      }
      const svgTM = d3.select('#ThreadMapMain');
      const backgroundTooltip = d3
      .select('#ThreadMapMain')
      .append('rect')
      .attr('class', 'tooltip-background-tm')
      .style('opacity', 0);
      const textTooltip = d3.select('#ThreadMapMain').append('text').attr('class', 'tooltip-text-tm').style('opacity', 0);
      const ProcessIDForceGraph = d3.select('#ThreadMapMain').append('text').attr('class', 'Forcegraph').attr('ProcessID', 'select').style('opacity', 0);
      //Adjust width
      if (DataLength*20+20 < width) {
        d3.select('#ThreadMapMain')
        .attr('width', width);
      }
      else {
      d3.select('#ThreadMapMain')
      .attr('width', DataLength*20+20);
      }
      //Draw Separator
      svgTM
      .selectAll('rect')
        .data(SepparatorData)
        .enter()
        .append('rect')
        .attr('x', function (d: any) {
          return d.x;
        })
        .attr('y', function (d: any) {
          return d.y;
        })
        .attr('width', function (d: any) {
          return d.width;
        })
        .attr('height', function (d: any) {
          return d.height;
        })
        .style('fill', function (d: any) {
          return d.colour;
        })
      //Draw ThreadMap
      svgTM
      .selectAll('circle')
        .data(MapData)
        .enter()
        .append('circle')
        .attr('cx', function (d: any) {
          return d.cx;
        })
        .attr('cy', function (d: any) {
          return d.cy;
        })
        .attr('r', function (d: any) {
          return d.r;
        })
        .attr('class', function (d: any) {
          return d.class;
        })
        .attr('name', function (d: any) {
          return d.name;
        })
        .style('fill', function (d: any) {
          return d.colour;
        })
        .attr('stroke', function (d: any) {
          return d.stroke;     
        })
        .attr('stroke-width', 2)

        .on('mouseover', function (d) {
          d3.select(this).transition().duration(50).attr('opacity', '.85');
          backgroundTooltip.transition().duration(50).style('opacity', 1);
          textTooltip.transition().duration(50).style('opacity', 1);
          //Define width of backgrounTooltip
          //Check for longest Element
          let bckgrndTT = Math.max((d.prefix.length + d.name.length), d.wrbytes.length)
          //Position of Tooltip, default topright
          let xtspan = 0
          if((d.cx + 20 + bckgrndTT * 8) <= width && (d.cy - 40) >= 0 ) {
            backgroundTooltip
              .attr('x', d.cx + 15)
              .attr('y', d.cy - 40 )
            textTooltip
              .attr('x', d.cx + 20)
              .attr('y', d.cy - 24)           
            //move Tooltip up
            if((d.cy -40 + (d.affiliated.length+3)* 20) + 4 >= (height-100)) {
              if((d.cy - (d.affiliated.length+3)* 20 + 4) < 0) {
                backgroundTooltip
                .attr('y', 0)
              textTooltip
                .attr('y', 20)
              }
              else {
              backgroundTooltip
                .attr('y', d.cy - (d.affiliated.length+3)* 20 + 4)
              textTooltip
                .attr('y', d.cy - (d.affiliated.length+2)* 20)
              }
            }
            xtspan = d.cx + 20
          }
          //Tooltip bottomleft
          else {
            backgroundTooltip
              .attr('x', d.cx - 5 - bckgrndTT * 7)
              .attr('y', d.cy + 10)
            textTooltip
              .attr('x', d.cx - 2 - bckgrndTT * 7)
              .attr('y', d.cy + 26)
            //move Tooltip up
            if((d.cy + 10 + (d.affiliated.length+3)* 20) + 4 >= (height-100)) {
              backgroundTooltip
              .attr('y', d.cy  + 10 - ((d.cy + 20 + (d.affiliated.length+2)* 20 + 4) - (height-100)))
              textTooltip
                .attr('y', d.cy + 26 - ((d.cy + 20 + (d.affiliated.length+2)* 20 + 4) - (height-100)))
            }
            xtspan = d.cx - 2 - bckgrndTT * 7
          }
          //(d.name.length+ d.prefix.length) * 8 + 3
          backgroundTooltip
            .attr('width', bckgrndTT * 6 + 3)
            .attr('height', (d.affiliated.length+3)* 16 + 4)
            .attr('rx', 3)
            .attr('fill', '#535353'); //circle colour or neutral? d.colour
          textTooltip
            .attr('font-family', 'Arial')
            .attr('font-size', 12)
            .attr('fill', 'white')
            .append('tspan')
              .text(d.prefix + d.name)
              .style('font-weight', 'bold')
            .append('tspan')
              .text(d.wrbytes)
              .style('font-weight', 'normal')
              .attr('x', xtspan)
              .attr('dy', '1.4em')
            .append('tspan')
              .text(d.affiliatedPrefix)
              .style('font-weight', 'normal')
              .attr('x', xtspan)
              .attr('dy', '1.2em')
          for (let i = 0; i < d.affiliated.length; i++) {
            textTooltip
              .append('tspan')
              .text(d.affiliated[i])
              .attr('x', xtspan)
              .attr('dy', '1.2em');              
          }
          //Put Tooltip in the front
          backgroundTooltip.raise();
          textTooltip.raise();
        })
        .on('mouseout', function (d) {
          d3.select(this).transition().duration(50).attr('opacity', '1');
          backgroundTooltip.transition().duration(50).style('opacity', 0);
          textTooltip.transition().duration(50).style('opacity', 0);
          //remove old hover
          textTooltip.selectAll('tspan').remove();
          //Put Tooltip in the back
          backgroundTooltip.lower();
          textTooltip.lower();
        })
        .on('click', function(d) {
          if(d.class === "Process") {
          ProcessIDForceGraph
          .attr('ProcessID', d.name)
          //Remove old highlight
          d3.selectAll('circle')
          .filter(function() {return d3.select(this).attr('r') === '8';})
          .attr('stroke', null);
          //Add Highliting
          d3.select(this).attr('stroke', 'blue').attr('stroke-width', 2);
          }
        });
        //Highlight Threads with filtered Filenames
        //Remove old highlight
        d3.selectAll('circle')
          .filter(function() {return d3.select(this).attr('r') === '6';})
          .attr('stroke', null);
        d3.selectAll('circle')
        .filter(function() {return d3.select(this).attr('r') === '6';})
        .attr('stroke', function (d: any) {
            return d.stroke;     
        })
        .attr('stroke-width', 2)
        //Style MinMax Display
        //remove old min max
        d3.select('#minValueText').remove();
        d3.select('#maxValueText').remove();
        d3.select('#maxValueColour').remove();
        const svgMinMax = d3.select('#ThreadMapMinMax')
        const drawminVal = d3.select('#ThreadMapMinMax').append('text').attr('class', 'minValue')
        const drawminValColour = d3.select('#ThreadMapMinMax').append('circle').attr('class', 'minValue')
        const drawmaxVal = d3.select('#ThreadMapMinMax').append('text').attr('class', 'maxValue')
        const drawmaxValColour = d3.select('#ThreadMapMinMax').append('circle').attr('class', 'maxValue')
        svgMinMax
          .attr('width', DataLength*40)
        drawminVal
          .attr('id', 'minValueText')
          .attr('x', 30)
          .attr('y', 67)
          .attr('font-family','Arial')
          .attr('font-size', 20)
          .attr('fill', 'White')
          .attr('font-weight', 'bold')
          .html(minVal.toString())
        drawminValColour
          .attr('cx', 15)
          .attr('cy', 60)
          .attr('r', 10)
          .style('fill', '#00FF00')
        drawmaxVal
          .attr('id', 'maxValueText')
          .attr('x', 84 + minValLength)
          .attr('y', 67)
          .attr('font-family','Arial')
          .attr('font-size', 20)
          .attr('fill', 'White')
          .attr('font-weight', 'bold')
          .html(maxVal.toString())
        drawmaxValColour
          .attr('id', 'maxValueColour')
          .attr('cx', 69 + minValLength)
          .attr('cy', 60)
          .attr('r', 10)
          .style('fill', '#FF0000')
        const ProcessText = d3.select('#ThreadMapMain').append('text')
        ProcessText
          .attr('x', 5)
          .attr('y', 30)
          .attr('font-family','Arial')
          .attr('font-size', 20)
          .attr('fill', 'White')
          .attr('font-weight', 'bold')
          .html('Process:')
          const ThreadText = d3.select('#ThreadMapMain').append('text')
          ThreadText
            .attr('x', 5)
            .attr('y', 110)
            .attr('font-family','Arial')
            .attr('font-size', 18)
            .attr('fill', 'White')
            .attr('font-weight', 'bold')
            .html('Threads:')
    }
    
  }, [options, data, height, width]);

  return(
    <div>
      <CustomScrollbar>
        <svg id="ThreadMapMain" height={height-100}>
        </svg>
      </CustomScrollbar>
      <svg id="ThreadMapMinMax">
        <text x={(5)} y= {(30)} fontFamily='Arial' fontSize='20' fill='White' fontWeight='bold'>Colour by written & read bytes threads: (min / max)</text>
      </svg>
      </div>
  )
}
