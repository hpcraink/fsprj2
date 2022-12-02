import React, { useEffect } from 'react'; //useEffect
import { PanelProps } from '@grafana/data';
import { SimpleOptions } from 'types';

interface Props extends PanelProps<SimpleOptions> {}

export const ThreadMap: React.FC<Props> = ({ options, data, width, height }) => {
  var ProcessIDArrCss = new Array //Css Daten für Grafana
  var ProcessIDArr = new Array    //Speicher ProzessNamen, Hilfvariable um doppelte Prozessnamen zu filtern
  var ThreadIDArrCss = new Array
 
  useEffect(() =>{ //useEffect notwendig oder anderen Operator?

  },[data, height, width]);

  //Generieren der Css-Daten für alle Prozesse & Threads
  for (let i = 0;i < data.series.length; i++) {
    //Abfangen doppelter Threads
    if(!(ProcessIDArr.includes(data.series[i].fields[1].labels?.processid))) {
      ProcessIDArrCss[i] = BuildPanelProcess(i)
    }
    ProcessIDArr[i] = data.series[i].fields[1].labels?.processid
    ThreadIDArrCss[i] = BuildPanelThread(i)
  }

  function BuildPanelProcess(i: number)
  {
      return (
      <g>
      <rect x={(140*i)} y={(0)} width={135} height={50} rx="10"/>
      <text x={(5+140*i)} y= {(40)} font-family="Arial" font-size="30" fill="blue">{data.series[i].fields[1].labels?.processid}</text>
      </g>
      );
  }

  function BuildPanelThread(i: number)
  {
    return (
    <g>
    <rect x={(140*i)} y={(100)} width={135} height={50} rx="10"/>
    <text x={(5+140*i)} y= {(140)} font-family="Arial" font-size="20" fill="blue">{data.series[i].fields[1].labels?.thread}</text>
    </g>
    );
  }

  return(
    <div>
      <svg width={width} height={height}>
        {ProcessIDArrCss}
        {ThreadIDArrCss}
      </svg>
    </div>
  );
};