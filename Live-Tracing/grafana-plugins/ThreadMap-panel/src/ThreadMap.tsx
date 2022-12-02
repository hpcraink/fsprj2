import React, { useEffect } from 'react'; //useEffect
import { PanelProps } from '@grafana/data';
import { SimpleOptions } from 'types';

//import * as d3 from 'd3';
//import { svg } from 'd3';

//var Teststring = "<div></div>"
interface Props extends PanelProps<SimpleOptions> {}


export const ThreadMap: React.FC<Props> = ({ options, data, width, height }) => {
  var ProcessIDArr = new Array
  var ThreadIDArr = new Array
 
  useEffect(() =>{ //useEffect notwendig oder anderen Operator?
    
    //Auslesen der Prozesse/Threads
    // function ReadData()
    // {
    //   console.log(data.series)
    //   console.log(data.series[1].fields[1].labels)
    // }

    //Aufrufe
    //ReadData();  


},[data, height, width]);
  let i = 0

  for (;i < data.series.length; i++) {
    ProcessIDArr[i] = BuildPanelProcess(i)
    ThreadIDArr[i] = BuildPanelThread(i)
  }
  //console.log(data.series.length)
  //console.log(data.series[1].fields[1].labels)

  function BuildPanelProcess(i: number)
  {
      //console.log(data.series[i].fields[1].labels?.processid, i+1)
    return (
          <g>
          <rect x={(140*i)} y={(0)} width={135} height={50} rx="10"/>
          <text x={(5+140*i)} y= {(40)} font-family="Arial" font-size="30" fill="blue">{data.series[i].fields[1].labels?.processid}</text>
          </g>
    );
  }

  function BuildPanelThread(i: number)
  {
    console.log(data.series[i].fields[1].labels?.thread, i+1)
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
        {ProcessIDArr}
        {ThreadIDArr}
      </svg>
    </div>
  );
};