import React, {useEffect} from 'react';
import { PanelProps } from '@grafana/data';
import { PanelOptions } from 'options';
import _ from 'lodash';
import { CustomScrollbar } from '@grafana/ui' //npnmistall @grafana/ui


interface ThreadMapPanelProps extends PanelProps<PanelOptions> {}

export const ThreadMap: React.FC<ThreadMapPanelProps> = ({ 
  options, data, height 

}) => {

  var ProcessIDArrCss = new Array //Css Daten für Grafana
  var ProcessColour = new Array   //Farbe Auslastung Prozesse
  var ThreadIDArrCss = new Array  //Css Daten für Grafana
  var ThreadColour = new Array    //Farbe der Auslastung Threads
  var Colours = new Array         //Anzahl möglicher Farben

  useEffect(() =>{ //useEffect notwendig oder anderen Operator?
    options
  },[data, height]);

  //Färbung der Anhand deren Auslastung
  Colours = ColourSteps()
  const ColAssig = ColourAssignment(Colours)
  ThreadColour = ColAssig[0]
  ProcessColour = ColAssig[1]

  //Generieren der Css-Daten für alle Prozesse & Threads
  for (let i = 0;i < data.series.length; i++) {
    if(ProcessColour[i] !== undefined)
    {
      ProcessIDArrCss[i] = BuildPanelProcess(i, ProcessColour[i])
    }
    //ProcessIDArr[i] = data.series[i].fields[1].labels?.processid
    ThreadIDArrCss[i] = BuildPanelThread(i, ThreadColour[i])
  }
  
  //Funktionen
  function ColourSteps()
  {
    let Colour = new Array
    const step = 255 / ((data.series.length-1)*0.5)
    Colour[0] = "#00ff00"

    for (let i = 1; i < (data.series.length); i++) {
      if(i < (data.series.length/2)) { //Grün #00FF00 bis Gelb #FFFF00
        if ((i*step) < 16) {
          Colour[i] = "#0" + (i*step).toString(16).substring(0,1) + "ff00"
        }
        else {
        Colour[i] = "#" + (i*step).toString(16).substring(0,2) + "ff00"
        }
      }
      else { //Gelb #FFFF00 bis Rot #FF0000 
        if (((i-data.series.length/2)*step) >= 240) {
          Colour[i] = "#ff0" + (255-((i-data.series.length/2)*step)).toString(16).substring(0,1) + "00"
        }
        else {
        Colour[(i)] = "#ff" + (255-((i-data.series.length/2)*step)).toString(16).substring(0,2) + "00"
        }
      }
    }
    return(Colour)
  }
  
  function ColourAssignment(Colour: any) {
    var ThreadHeatValue = new Array
    var ProcessHeatValue = new Array
    var ReturnColourThreads = new Array
    var ReturnColourProcess = new Array

    for (let i = 0; i < data.series.length; i++) {
      //data.series[i].fields[1].values.buffer[0] //nicht verwendbar? => ThreadHeatValue.buffer[i] als Workaround
      ThreadHeatValue[i] =data.series[i].fields[1].values
      ThreadHeatValue[i] = _.sum(ThreadHeatValue[i].buffer)
    }

    for (let iproc = 0; iproc < data.series.length; iproc++) {
      if ((iproc < (data.series.length - 1))) {
        if (!(data.series[iproc].fields[1].labels?.processid == data.series[iproc+1].fields[1].labels?.processid)) {
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

    function Assignment(HeatValue :any)
    {
      //remove undefined Values for min/max
      const MathValue = HeatValue.filter(function(element : any) {
        return element !== undefined;
     });
      let min = Math.min(...MathValue)
      let max = Math.max(...MathValue)
      let test = 0
      var ReturnColour = new Array
      let j = 0
      for (; HeatValue[j] >= (min+test*((max-min)/data.series.length)) && (j < data.series.length);) { 
        if (HeatValue[j] <= (min+(test+1)*((max-min)/data.series.length))){
          ReturnColour[j] = Colour[test]
          j++
          test=0
          if (HeatValue[j] == undefined) {
            j++
          }
        }
        else{
          test++
        }
      }
      return(ReturnColour);
    }
    return[ReturnColourThreads, ReturnColourProcess];

  }

  function BuildPanelProcess(i: number,Colour: any)
  {
    return (
    //horizontal
    <g>
    <rect x={(5+150*i)} y={(40)} width={135} height={50} rx="10" fill={Colour}/>
    <text x={(10+150*i)} y= {(80)} font-family="Arial" font-size="30" fill="black">{data.series[i].fields[1].labels?.processid}</text>
    <rect x={(8*i)} y={(110)} width={7} height={7} rx="10" fill={Colour}/>
    <rect x={(150*i-3)} y={(0)} width={3} height={height} fill="black"/>
    </g>

    //vertikal  
    // <g>
    // <rect x={(10)} y={(55*i)} width={135} height={50} rx="10" fill = {Colour} />
    // <text x={(15)} y= {(40+55*i)} font-family="Arial" font-size="30" fill="black">{data.series[i].fields[1].labels?.processid}</text>
    // </g>
    );
  }

  function BuildPanelThread(i: number,Colour: any)
  {
    return (
    //horizontal
    <g>
    <rect x={(5+150*i)} y={(160)} width={135} height={50} rx="10" fill={Colour}/>
    <text x={(10+150*i)} y= {(200)} font-family="Arial" font-size="20" fill="black">{data.series[i].fields[1].labels?.thread}</text>
    {/* <rect x={(8*i)} y={(230)} width={7} height={7} rx="10" fill={Colour}/> */}
    </g>

    //vertikal
    // <g>
    // <rect x={(195)} y={(55*i)} width={135} height={50} rx="10" fill={Colour}/>
    // <text x={(200)} y= {(40+55*i)} font-family="Arial" font-size="30" fill="black">{data.series[i].fields[1].labels?.processid}</text>
    // </g>
    );
  }


  return(
    //<div style={divStyle}>
    <div>
      <CustomScrollbar>
        <svg width={data.series.length*150} height={height}>
          <text x={(5)} y= {(30)} font-family="Arial" font-size="20" fill="White" font-weight="bold">Prozesse:</text>
          {ProcessIDArrCss}
          <text x={(5)} y= {(150)} font-family="Arial" font-size="20" fill="White" font-weight="bold">Threads:</text>
          {ThreadIDArrCss}    
        </svg>
      </CustomScrollbar>
    </div>
  );
};
