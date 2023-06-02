import React from 'react';
import { PanelProps } from '@grafana/data';
import { PanelOptions } from 'options';
import _ from 'lodash';
import { CustomScrollbar } from '@grafana/ui'; //npnmistall @grafana/ui
import * as d3 from 'd3';

interface ThreadMapPanelProps extends PanelProps<PanelOptions> {}

export const ThreadMap: React.FC<ThreadMapPanelProps> = ({ options, data, width, height}) => {
  //useEffect(() => {
    //var ProcessIDArrCss = new Array   //CSS Process
    var ProcessColour = new Array     
    //var ThreadIDArrCss = new Array    //Css Threads
    var ThreadColour = new Array      
    var Colours = new Array           //Possible Colour Amount
    //var FilesystemValue = new Array
    //var FilesystemCss = new Array
    var DataLength = 0                //Datalength libiotrace w/o filesystem
    var MapData : any = []

    //Amount Threads (written)
    if(DataLength == 0) {
      for (let i = 0; i < data.series.length; i++) { 
        if ((data.series[i].fields[1] !== undefined) && (data.series[i].refId === "Written Bytes")) {
          DataLength++
        }
      }
    }

    //Coloring based on amonut written bytes
    Colours = ColourSteps(DataLength)
    const ColAssig = ColourAssignment(Colours, DataLength)
    ThreadColour = ColAssig[0]?.[0]
    ProcessColour = ColAssig[1]
    //FilesystemValue = FilesystemAssignment()

    //Generate CSS-data for process & threads
    //let j = 0
    let xShift = 0, k = 0
    for (let i = 0;i < DataLength; i++) {
      if(ProcessColour[i] !== undefined) 
      {
        BuildPanel(i, k, xShift, ProcessColour[i], 8, 60)
        k++
        
      }
      BuildPanel(i, k, xShift, ThreadColour[i], 6, 140)
      k++
      //Filesystem
      // let yPlacement = 0, tShift = 0
      // for (let bufferCnt = 0; bufferCnt < FilesystemValue[i].length; bufferCnt++) {
      //   yPlacement = bufferCnt - (5*tShift)
      //   FilesystemCss[j] = BuildPanelThreadBuffer_FileSystem(i, yPlacement, xShift, FilesystemValue[i][bufferCnt])
      //   j++
      //   if ((yPlacement > 3) && (bufferCnt+1 < data.series[i].fields[1].values.length) ) {
      //     xShift++
      //     tShift++
      //   }
      // }
    }

    //Call Tooltip
    drawTooltip()

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
        return[ReturnColour, min, max]
      }
      //Return Min/Max only for Threads
      return[ReturnColourThreads, ReturnColourProcess[0]]
    }

    //function FilesystemAssignment() {
      // //Transfer traced_filename to local var
      // var lTracedFilename = new Array
      // let lOutputFs: any[][] = [[]];
      // for (let i = 0; i < data.series.length-DataLength; i++) {
      //   lTracedFilename[i] = data.series[i+DataLength].fields[1].values
      // }
      
      //convert filename to filesystem
    //   for (let i = 0; i < lTracedFilename.length; i++) {
    //     const lFileSystem: string[] = [];
    //     for (let j = 0; j < lTracedFilename[i].buffer.length; j++) {
    //       const pathSegments = lTracedFilename[i].buffer[j].split("/");
    //       if (pathSegments[1] !== undefined) {
    //         if(!lFileSystem.includes(pathSegments[1])) {
    //           lFileSystem.push(pathSegments[1])
    //         }
    //       }
    //       else {
    //         if (!lFileSystem.includes(pathSegments[0])) {
    //         lFileSystem.push(pathSegments[0])
    //         }
    //       }
    //     }
    //     lOutputFs.push(lFileSystem)
    //   }
    //   //delete first Element
    //   lOutputFs.shift();
    //   return(lOutputFs)
    // }

    function drawTooltip() {
      const svg = d3.select('#ThreadMapMain');
      const backgroundTooltip = d3
          .select('#ThreadMapMain')
          .append('rect')
          .attr('class', 'tooltip-background')
          .style('opacity', 0);
        const textTooltip = d3.select('#ThreadMapMain').append('text').attr('class', 'tooltip-text').style('opacity', 0);
      //Draw
      svg
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
        .style('fill', function (d: any) {
          return d.colour;
        })
        .on('mouseover', function (d) {
          d3.select(this).transition().duration(50).attr('opacity', '.85');
          backgroundTooltip.transition().duration(50).style('opacity', 1);
          textTooltip.transition().duration(50).style('opacity', 1);
          backgroundTooltip
            .attr('x', d.cx + 15)
            .attr('y', d.cy - 40)
            .attr('width', d.name.length * 10)
            .attr('height', '25')
            .attr('rx', '3')
            .attr('fill', d.colour);
          textTooltip
            .attr('x', d.cx + 20)
            .attr('y', d.cy - 20)
            .attr('font-family', 'Arial')
            .attr('font-size', '15')
            .attr('fill', 'black')
            .html(d.name);
        })
        .on('mouseout', function (d) {
          d3.select(this).transition().duration(50).attr('opacity', '1');
          backgroundTooltip.transition().duration(50).style('opacity', 0);
          textTooltip.transition().duration(50).style('opacity', 0);
        });
        //Put Tooltip in the front
        backgroundTooltip.raise();
        textTooltip.raise();
    }

    function BuildPanel(i: number, k: number, lxShift: number,Colour: any, r: number, cy: number)
    {
      MapData.push(
        {
          index: k,
          name: data.series[i].fields[1].labels?.thread,
          cx:(15+20*(i+lxShift)),
          cy: cy,
          r: r,
          colour: Colour
        }
      )
    }

    // function BuildPanelThreadBuffer_FileSystem(i: number, yPlacement: number, lxShift : number, lFilesystemValue: any)
    // {
    //   return(
    //     <g className = "Filesystem">
    //       <text x={(20+40*(i+lxShift))} y= {(311+30*yPlacement)} fontFamily='Arial' fontSize='14' fill='white'>{lFilesystemValue}</text>
    //     </g>
    //   )
    // }
  //}, [options, data, height, width]);

  //Put text into onmousehover?
  return(
    <div>
      <CustomScrollbar>
        <svg id="ThreadMapMain" width={(DataLength+xShift)*20+20} height={height-100}>
          <text x={(5)} y= {(30)} fontFamily='Arial' fontSize='20' fill='White' fontWeight='bold'>Prozess:</text>
          <text x={(5)} y= {(110)} fontFamily='Arial' fontSize='18' fill='White' fontWeight='bold'>Threads:</text>
          {/* <text x={(5)} y= {(270)} fontFamily='Arial' fontSize='20' fill='White' fontWeight='bold'>Filesystem:</text> */}
          {/* {FilesystemCss} */}
        </svg>
      </CustomScrollbar>
      <svg id="ThreadMapMinMax" width={(DataLength+xShift)*40}>
        <text x={(5)} y= {(30)} fontFamily='Arial' fontSize='20' fill='White' fontWeight='bold'>Colour by written bytes Threads: (Min / Max)</text>
        <rect x={(5)} y={(50)} width={20} height={20} rx='10' fill='#00FF00'/>
        <text x={(30)} y= {(67)} fontFamily='Arial' fontSize='20' fill='White' fontWeight='bold'>{minVal}</text>
        <rect x={(59 + minValLength)} y={(50)} width={20} height={20} rx='10' fill='#FF0000'/>         
        <text x={(84 + minValLength)} y= {(67)} fontFamily='Arial' fontSize='20' fill='White' fontWeight='bold'>{maxVal}</text>
      </svg>
      </div>
  )
}
