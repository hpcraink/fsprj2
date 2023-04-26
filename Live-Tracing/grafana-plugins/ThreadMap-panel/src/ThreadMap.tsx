import React from 'react';
import { PanelProps } from '@grafana/data';
import { PanelOptions } from 'options';
import _ from 'lodash';
import { CustomScrollbar } from '@grafana/ui'; //npnmistall @grafana/ui

interface ThreadMapPanelProps extends PanelProps<PanelOptions> {}

export const ThreadMap: React.FC<ThreadMapPanelProps> = ({ options, data, width, height}) => {
  var ProcessIDArrCss = new Array   //Css Daten für Grafana
  var ProcessColour = new Array     //Farbe Auslastung Prozesse
  var ThreadIDArrCss = new Array    //Css Daten für Grafana
  var ThreadColour = new Array      //Farbe der Auslastung Threads
  var Colours = new Array           //Anzahl möglicher Farben
  var FilesystemValue = new Array
  var ThreadBufferColour = new Array
  var ThreadBufferCss = new Array
  var DataLength = 0                //Datenlänge libiotrace ohne filesystem

  //Anzahl echter Datenpunkte
  if(DataLength == 0) {
    for (let i = 0; i < data.series.length; i++) { 
      if ((data.series[i].fields[1] !== undefined) && (data.series[i].fields[1].name === "function_data_written_bytes")) {
        DataLength++
      }
    }
  }

  //Färbung der Anhand deren Auslastung
  Colours = ColourSteps(DataLength)
  const ColAssig = ColourAssignment(Colours, DataLength)
  ThreadColour = ColAssig[0]?.[0]
  ProcessColour = ColAssig[1]
  ThreadBufferColour = ColAssig[2]
  FilesystemValue = FilesystemAssignment()

  //Generieren der Css-Daten für alle Prozesse & Threads
  let j = 0, xShift = 0
  for (let i = 0;i < DataLength; i++) {
    if(ProcessColour[i] !== undefined) 
    {
      ProcessIDArrCss[i] = BuildPanelProcess(i, xShift, ProcessColour[i])
    }
    ThreadIDArrCss[i] = BuildPanelThread(i, xShift, ThreadColour[i])
    let yPlacement = 0, tShift = 0
    for (let bufferCnt = 0; bufferCnt < data.series[i].fields[1].values.length; bufferCnt++) {
      yPlacement = bufferCnt - (5*tShift)
      ThreadBufferCss[j] = BuildPanelThreadBuffer_FileSystem(i, yPlacement, xShift, ThreadBufferColour[j], FilesystemValue[i].buffer[bufferCnt])
      j++
      if ((yPlacement > 3) && (bufferCnt+1 < data.series[i].fields[1].values.length) ) {
        xShift++
        tShift++
      }
    }
    
  }

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

  //Funktionen
  function ColourSteps(lDdatalength: any)
  {
    let Colour = new Array
    const step = 255 / ((lDdatalength-1)*0.5)
    Colour[0] = '#00ff00'

    for (let i = 1; i < (lDdatalength); i++) {
      if(i < (lDdatalength/2)) { //Grün #00FF00 bis Gelb #FFFF00
        if ((i*step) < 16) {
          Colour[i] = '#0' + (i*step).toString(16).substring(0,1) + 'ff00'
        }
        else {
        Colour[i] = '#' + (i*step).toString(16).substring(0,2) + 'ff00'
        }
      } 
      else { //Gelb #FFFF00 bis Rot #FF0000 
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
    var HelpArrayDl = new Array
    var ThreadHeatValue = new Array
    var ProcessHeatValue = new Array
    var BufferHeatValue = new Array
    var ReturnColourThreads = new Array
    var ReturnColourProcess = new Array
    var ReturnColourThreadBuffer = new Array
    let bufferIndex = 0

    for (let i = 0; i < lDatalength; i++) {
      HelpArrayDl[i] = data.series[i].fields[1].values
      ThreadHeatValue[i] = _.sum(HelpArrayDl[i].buffer)
      for (let j = 0; j < HelpArrayDl[i].buffer.length; j++) {
        BufferHeatValue[bufferIndex] = HelpArrayDl[i].buffer[j]
        bufferIndex++
      }
    }

    for (let iproc = 0; iproc < lDatalength; iproc++) {
      if ((iproc < (lDatalength - 1))) {
        if (!(data.series[iproc].fields[1].labels?.processid === data.series[iproc+1].fields[1].labels?.processid)) {
          ProcessHeatValue[iproc] = ThreadHeatValue[iproc]
        }
        else{
          ProcessHeatValue[iproc] = ThreadHeatValue[iproc] + ThreadHeatValue[iproc+1]
          iproc++
        }
      }
      else{
        ProcessHeatValue[iproc] = ThreadHeatValue[iproc]
      }
    }
    ReturnColourThreads = Assignment(ThreadHeatValue)
    ReturnColourProcess = Assignment(ProcessHeatValue)
    ReturnColourThreadBuffer = Assignment(BufferHeatValue)

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
            if (HeatValue[j] === undefined) {
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
            if (HeatValue[j] === undefined) {
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
    //MinMax hier nur für Threads zurückgegeben
    return[ReturnColourThreads, ReturnColourProcess[0], ReturnColourThreadBuffer[0]]
  }

  function FilesystemAssignment(){
    var lFilesystemValue = new Array
    for (let i = 0; i < data.series.length-DataLength; i++) {
      lFilesystemValue[i] = data.series[i+DataLength].fields[1].values
    }

    //Only write filesystem
    for (let i = 0; i < lFilesystemValue.length; i++) {
      for (let j = 0; j < lFilesystemValue[i].buffer.length; j++) {
        lFilesystemValue[i].buffer[j] = lFilesystemValue[i].buffer[j].split(["/"])
        if (lFilesystemValue[i].buffer[j][1] !== undefined) {
          lFilesystemValue[i].buffer[j] = lFilesystemValue[i].buffer[j][1]
        }
        else{
          lFilesystemValue[i].buffer[j] = lFilesystemValue[i].buffer[j][0]
        }
      }
    }

    //Sort?
    
    //Assignment
  

    //Incorrect Assignment
    // let j = 0,k = 0
    // for (let i = 0; i < DataLength; i++) {
    //   lFilesystemValue[i] = HelpArr[j].buffer[k]
    //   lFilesystemValue[i] = lFilesystemValue[i].split(["/"])
    //   if (lFilesystemValue[i][1] !== undefined) {
    //     lFilesystemValue[i] = lFilesystemValue[i][1]
    //   }
    //   k++
    //   if (HelpArr[j].buffer[k] === undefined) 
    //   {
    //     j++
    //     if (j >= data.series.length-DataLength) {
    //       break
    //     }
    //     k=0
    //   }
    // }
    return(lFilesystemValue)
  }

  function BuildPanelProcess(i: number, lxShift: number,Colour: any)
  {
    return (
    //horizontal
    <g>
    <rect x={(5+160*(i+lxShift))} y={(50)} width={135} height={50} rx='10' fill={Colour}/>
    <text x={(10+160*(i+lxShift))} y= {(90)} fontFamily='Arial' fontSize='30' fill='black'>{data.series[i].fields[1].labels?.processid}</text>
    <rect x={(160*(i+lxShift)-3)} y={(0)} width={3} height={height-150} fill='black'/>
    </g>

    //vertikal
    // <g>
    // <rect x={(10)} y={(55*i)} width={135} height={50} rx='10' fill = {Colour} />
    // <text x={(15)} y= {(40+55*i)} fontFamily='Arial' fontSize='30' fill='black'>{data.series[i].fields[1].labels?.processid}</text>
    // </g>
    )
  }

  function BuildPanelThread(i: number, lxShift: number,Colour: any)
  {
    return(
    //horizontal
    <g>
    <rect x={(5+160*(i+lxShift))} y={(170)} width={135} height={50} rx='10' fill={Colour}/>
    <text x={(10+160*(i+lxShift))} y= {(210)} fontFamily='Arial' fontSize='20' fill='black'>{data.series[i].fields[1].labels?.thread}</text>
    </g>

    //vertikal
    // <g>
    // <rect x={(195)} y={(55*i)} width={135} height={50} rx='10' fill={Colour}/>
    // <text x={(200)} y= {(40+55*i)} fontFamily='Arial' fontSize='30' fill='black'>{data.series[i].fields[1].labels?.processid}</text>
    // </g>
    )
  }

  function BuildPanelThreadBuffer_FileSystem(i: number, yPlacement: number, lxShift : number,Colour: any, lFilesystemValue: any)
  {
    return(
      //horizontal
      <g>
        <rect x={(20+160*(i+lxShift))} y={(300+30*yPlacement)} width={16} height={16} rx='8' fill={Colour}/>
        <text x={(45+160*(i+lxShift))} y= {(311+30*yPlacement)} fontFamily='Arial' fontSize='14' fill='white'>{lFilesystemValue}</text>
      </g>
    )
  }

  return(
    <div>
      <CustomScrollbar>
        <svg width={(DataLength+xShift)*150} height={height-100}>
          <text x={(5)} y= {(30)} fontFamily='Arial' fontSize='20' fill='White' fontWeight='bold'>ProzessID:</text>
          <text x={(5)} y= {(150)} fontFamily='Arial' fontSize='20' fill='White' fontWeight='bold'>ThreadID:</text>
          <text x={(5)} y= {(270)} fontFamily='Arial' fontSize='20' fill='White' fontWeight='bold'>Filesystem:</text>
          {ProcessIDArrCss}
          {ThreadIDArrCss}
          {ThreadBufferCss}
        </svg>
      </CustomScrollbar>
      <svg width={(DataLength+xShift)*150}>
        <text x={(5)} y= {(30)} fontFamily='Arial' fontSize='20' fill='White' fontWeight='bold'>Colour by written bytes Threads: (Min / Max)</text>
        <rect x={(5)} y={(50)} width={20} height={20} rx='10' fill='#00FF00'/>
        <text x={(30)} y= {(67)} fontFamily='Arial' fontSize='20' fill='White' fontWeight='bold'>{minVal}</text>
        <rect x={(59 + minValLength)} y={(50)} width={20} height={20} rx='10' fill='#FF0000'/>         
        <text x={(84 + minValLength)} y= {(67)} fontFamily='Arial' fontSize='20' fill='White' fontWeight='bold'>{maxVal}</text>
      </svg>
      </div>
  )
}
