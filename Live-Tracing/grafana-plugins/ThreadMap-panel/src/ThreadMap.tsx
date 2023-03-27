import React, { useEffect } from 'react';
import { PanelProps } from '@grafana/data';
import { PanelOptions } from 'options';
import _ from 'lodash';
import { CustomScrollbar } from '@grafana/ui'; //npnmistall @grafana/ui
export var minValue: any  //Test for Settings

interface ThreadMapPanelProps extends PanelProps<PanelOptions> {}

export const ThreadMap: React.FC<ThreadMapPanelProps> = ({ options, data, height }) => {
  var ProcessIDArrCss = new Array   //Css Daten für Grafana
  var ProcessColour = new Array     //Farbe Auslastung Prozesse
  var ThreadIDArrCss = new Array    //Css Daten für Grafana
  var ThreadColour = new Array      //Farbe der Auslastung Threads
  var Colours = new Array           //Anzahl möglicher Farben
  var FilesystemValue = new Array
  var FilesystemArrCSS = new Array  //Css
  var DataLenght = 0                //Datenlänge libiotrace ohne filesystem

  useEffect(() =>{ //useEffect notwendig oder anderen Operator?
    options
  },[data, height])

  //Anzahl echter Datenpunkte
  if(DataLenght == 0)
  {
    for (let i = 0; i < data.series.length; i++) { 
      if (data.series[i].name === "libiotrace") {
        DataLenght++
      }
    }
  }

  //Färbung der Anhand deren Auslastung
  Colours = ColourSteps(DataLenght)
  const ColAssig = ColourAssignment(Colours, DataLenght)
  ThreadColour = ColAssig[0]?.[0]
  minValue = ColAssig[0]?.[1]
  ProcessColour = ColAssig[1]
  FilesystemValue = FilesystemAssignment()

  //Generieren der Css-Daten für alle Prozesse & Threads
  //let j = 0
  for (let i = 0;i < DataLenght; i++) {
    if(ProcessColour[i] !== undefined) 
    {
      ProcessIDArrCss[i] = BuildPanelProcess(i, ProcessColour[i])
      //FilesystemArrCSS[i] = BuildPanelFilesystem(i, j)
      //j++;
    }
    //ProcessIDArr[i] = data.series[i].fields[1].labels?.processid
    ThreadIDArrCss[i] = BuildPanelThread(i, ThreadColour[i])
    FilesystemArrCSS[i] = BuildPanelFilesystem(i,FilesystemValue )
  }

  //Funktionen
  function ColourSteps(lDdatalength: any)
  {
    let Colour = new Array
    const step = 255 / ((lDdatalength-1)*0.5)
    Colour[0] = "#00ff00"

    for (let i = 1; i < (lDdatalength); i++) {
      if(i < (lDdatalength/2)) { //Grün #00FF00 bis Gelb #FFFF00
        if ((i*step) < 16) {
          Colour[i] = "#0" + (i*step).toString(16).substring(0,1) + "ff00"
        }
        else {
        Colour[i] = "#" + (i*step).toString(16).substring(0,2) + "ff00"
        }
      }
      else { //Gelb #FFFF00 bis Rot #FF0000 
        if (((i-lDdatalength/2)*step) >= 240) {
          Colour[i] = "#ff0" + (255-((i-lDdatalength/2)*step)).toString(16).substring(0,1) + "00"
        }
        else {
        Colour[(i)] = "#ff" + (255-((i-lDdatalength/2)*step)).toString(16).substring(0,2) + "00"
        }
      }
    }
    return(Colour)
  }
  
  function ColourAssignment(Colour: any, lDatalength: any) {
    var ThreadHeatValue = new Array
    var ProcessHeatValue = new Array
    var ReturnColourThreads = new Array
    var ReturnColourProcess = new Array

    for (let i = 0; i < lDatalength; i++) {
      //data.series[i].fields[1].values.buffer[0] //nicht verwendbar? => ThreadHeatValue.buffer[i] als Workaround
      ThreadHeatValue[i] =data.series[i].fields[1].values
      ThreadHeatValue[i] = _.sum(ThreadHeatValue[i].buffer)
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
      for (; HeatValue[j] >= (min+test*((max-min)/DataLenght)) && (j < DataLenght);) { 
        if (HeatValue[j] <= (min+(test+1)*((max-min)/DataLenght))){
          ReturnColour[j] = Colour[test]
          j++
          test = 0
          if (HeatValue[j] === undefined) {
            j++
          }
        }
        else{
          test++
        }
      }
      return[ReturnColour, min, max]
    }
    return[ReturnColourThreads, ReturnColourProcess[0]]
  }

  function FilesystemAssignment(){
    var lFilesystemValue = new Array
    var HelpArr = new Array
    for (let i = 0; i < data.series.length-DataLenght; i++) {
      HelpArr[i] =data.series[i+DataLenght].fields[1].values
    }
    let j = 0,k = 0
    for (let i = 0; i < DataLenght; i++) {
      lFilesystemValue[i] = HelpArr[j].buffer[k]
      k++
      if (HelpArr[j].buffer[k] === undefined) 
      {
        j++
        if (j >= data.series.length-DataLenght) {
          break
        }
        k=0
      }
    }
    return(lFilesystemValue)
  }

  function BuildPanelProcess(i: number,Colour: any)
  {
    return (
    //horizontal
    <g>
    <rect x={(5+150*i)} y={(50)} width={135} height={50} rx="10" fill={Colour}/>
    <text x={(10+150*i)} y= {(90)} fontFamily="Arial" fontSize="30" fill="black">{data.series[i].fields[1].labels?.processid}</text>
    {/* <rect x={(8*i)} y={(110)} width={7} height={7} rx="10" fill={Colour}/> */}
    <rect x={(150*i-3)} y={(0)} width={3} height={height-150} fill="black"/>
    </g>

    //vertikal
    // <g>
    // <rect x={(10)} y={(55*i)} width={135} height={50} rx="10" fill = {Colour} />
    // <text x={(15)} y= {(40+55*i)} fontFamily="Arial" fontSize="30" fill="black">{data.series[i].fields[1].labels?.processid}</text>
    // </g>
    )
  }

  function BuildPanelThread(i: number,Colour: any)
  {
    return (
    //horizontal
    <g>
    <rect x={(5+150*i)} y={(170)} width={135} height={50} rx="10" fill={Colour}/>
    <text x={(10+150*i)} y= {(210)} fontFamily="Arial" fontSize="20" fill="black">{data.series[i].fields[1].labels?.thread}</text>
    {/* <rect x={(8*i)} y={(230)} width={7} height={7} rx="10" fill={Colour}/> */}
    </g>

    //vertikal
    // <g>
    // <rect x={(195)} y={(55*i)} width={135} height={50} rx="10" fill={Colour}/>
    // <text x={(200)} y= {(40+55*i)} fontFamily="Arial" fontSize="30" fill="black">{data.series[i].fields[1].labels?.processid}</text>
    // </g>
    )
  }

  function BuildPanelFilesystem(i: number, lFilesystemValue: any)
  {
    return (
    <g>
      <text x={(20+150*i)} y= {(320)} fontFamily="Arial" fontSize="20" fill="white">{lFilesystemValue[i]}</text>
    </g>
    )
  }

  return(
    <div>
      <CustomScrollbar>
        <svg width={DataLenght*150} height={height-100}>
          <text x={(5)} y= {(30)} fontFamily="Arial" fontSize="20" fill="White" fontWeight="bold">ProzessID:</text>
          {ProcessIDArrCss}
          <text x={(5)} y= {(150)} fontFamily="Arial" fontSize="20" fill="White" fontWeight="bold">ThreadID:</text>
          {ThreadIDArrCss}
          <text x={(5)} y= {(270)} fontFamily="Arial" fontSize="20" fill="White" fontWeight="bold">Filesystem:</text>
          {FilesystemArrCSS}
        </svg>
      </CustomScrollbar>
      <svg width={DataLenght*150} height={height}>
          {/* Min Color + Min Value | Max Colour + MaxValue  | Colour Überlauf wie in HeatMap?*/}
          <text x={(5)} y= {(30)} fontFamily="Arial" fontSize="20" fill="White" fontWeight="bold">Written Bytes: (Min / Max)</text>
          <rect x={(5)} y={(50)} width={20} height={20} rx="10" fill="#00FF00"/>
          <text x={(30)} y= {(67)} fontFamily="Arial" fontSize="20" fill="White" fontWeight="bold">{ColAssig[0]?.[1]}</text>
          <rect x={(70)} y={(50)} width={20} height={20} rx="10" fill="#FF0000"/>         
          <text x={(95)} y= {(67)} fontFamily="Arial" fontSize="20" fill="White" fontWeight="bold">{ColAssig[0]?.[2]}</text>
          </svg>
    </div>
  )
}
