import React, {useEffect} from 'react'; //useEffect
import { PanelProps } from '@grafana/data';
import { SimpleOptions } from 'types';
import _ from 'lodash';
//import { CustomScrollbar } from '@grafana/ui' //npn istall @grafana/ui
//import { Component } from 'react';
//import ReactCustomScrollbars from 'react-custom-scrollbars-2';

interface Props extends PanelProps<SimpleOptions> {}

export const ThreadMap: React.FC<Props> = ({ options, data, width, height }) => {

  var ProcessIDArrCss = new Array //Css Daten für Grafana
  var ProcessIDArr = new Array    //Speicher ProzessNamen, Hilfvariable um doppelte Prozessnamen zu filtern
  var ThreadIDArrCss = new Array  //Css Daten für Grafana
  var ThreadHeatValue = new Array //Workaround für "Sum written_bytes per Thread"
  //var ThreadHeatValue2 = new Array
  var ThreadColour = new Array    //Farbverlauf der Auslastung

  useEffect(() =>{ //useEffect notwendig oder anderen Operator?

  },[data, height, width]);

  //Färbung der Prozesse Anhand deren Auslastung
  ColourSteps(ThreadColour)

  //Generieren der Css-Daten für alle Prozesse & Threads
  for (let i = 0;i < data.series.length; i++) {
    if(!(ProcessIDArr.includes(data.series[i].fields[1].labels?.processid)))     //Abfangen doppelter Threads
    {
      ProcessIDArrCss[i] = BuildPanelProcess(i)
    }
    ProcessIDArr[i] = data.series[i].fields[1].labels?.processid
    ThreadIDArrCss[i] = BuildPanelThread(i, ThreadColour[i])
  }
  
  //Funktionen
  function ColourSteps(ReturnColour: any)
  {
    var Colour = new Array
    const step = 255 / ((data.series.length-1)*0.5)
    Colour[0] = "#00FF00"

    for (let i = 1; i < (data.series.length); i++) {
      if(i < (data.series.length/2)) { //Grün #00FF00 bis Gelb #FFFF00
        if ((i*step) < 15) {
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
    
    for (let i = 0; i < data.series.length; i++) {
      //data.series[i].fields[1].values.buffer[0] nicht verwendbar? => ThreadHeatValue.buffer[i] als Workaround
      ThreadHeatValue[i] =data.series[i].fields[1].values
      ThreadHeatValue[i] = _.sum(ThreadHeatValue[i].buffer)
    }
    let j = 0
    let min = Math.min(...ThreadHeatValue)
    let max = Math.max(...ThreadHeatValue)
    let test = 0
    for (; ThreadHeatValue[j] >= (min+test*((max-min)/data.series.length)) && (j < data.series.length);) {   
      if (ThreadHeatValue[j] <= (min+(test+1)*((max-min)/data.series.length))){
        ReturnColour[j] = Colour[test]
        j++
        test=0
      }
      else{
        test++
      }
    }
    return(ReturnColour);
  }

  function BuildPanelProcess(i: number)
  {
    return (
    //horizontal
    <g>
    <rect x={(140*i)} y={(0)} width={135} height={50} rx="10" fill='green'/>
    <text x={(5+140*i)} y= {(40)} font-family="Arial" font-size="30" fill="black">{data.series[i].fields[1].labels?.processid}</text>
    </g>

    //vertikal  
    // <g>
    // <rect x={(10)} y={(55*i)} width={135} height={50} rx="10" fill = '#FF0000' />
    // <text x={(15)} y= {(40+55*i)} font-family="Arial" font-size="30" fill="black">{data.series[i].fields[1].labels?.processid}</text>
    // </g>
    );
  }

  function BuildPanelThread(i: number,Colour : any)
  {
    return (
    //horizontal
    <g>
    <rect x={(140*i)} y={(100)} width={135} height={50} rx="10" fill={Colour}/>
    <text x={(5+140*i)} y= {(140)} font-family="Arial" font-size="20" fill="black">{data.series[i].fields[1].labels?.thread}</text>
    <rect x={(8*i)} y={(170)} width={7} height={7} rx="10" fill={Colour}/>
    </g>


    //vertikal
    // <g>
    // <rect x={(195)} y={(55*i)} width={135} height={50} rx="10" fill={Colour}/>
    // <text x={(200)} y= {(40+55*i)} font-family="Arial" font-size="30" fill="black">{data.series[i].fields[1].labels?.processid}</text>
    // </g>
    );
  }

  console.log(ThreadHeatValue)
  return(
    <div>
      <div>

		  </div>
      <svg width={width} height={height}>
        {ProcessIDArrCss}
        {ThreadIDArrCss}      
      </svg>
    </div>
  );
};