import React, { useEffect } from 'react';
import { PanelProps } from '@grafana/data';
import { PanelOptions } from 'types';
import * as d3 from 'd3';
interface Props extends PanelProps<PanelOptions> {}

export const ForceFeedbackPanel: React.FC<Props> = ({ options, data, width, height }) => {
  useEffect(() => {
    let nodes: any = [];
    //sessionStorage.clear(); //Clear to erase beforehand data | not necessary w/o keeping old nodes
    let addingNodes = false;
    let links: any = [];
    let ProcessIndex = 0;
    //Get Nodeprocess from Plugin Settings
    let nodeProcess: any = options.ProcessID.toString();
    let nodeThreads: any = [];
    let nodeFileName: any = [];

    // We want old Nodes to be reused? Perfromance vs. might be wrong data
    //Check if changes are made that need sS to be cleared otherwise use it? => Livetest necessary
    // if (JSON.parse(sessionStorage.getItem('nodes')!) !== null) {
    //   nodes = JSON.parse(sessionStorage.getItem('nodes')!);
    // }

    function addNodes(_callback: any) {
      let nodeNumber = nodes.length;
      //Find Index of Process
      for (let i = 0; i < data.series.length / 2; i++) {
        if (data.series[i].fields[1].labels?.processid === nodeProcess) {
          ProcessIndex = i;
          break;
        }
      }
      //find all related threads and filenames | data.series.length/2 for second query
      let ctrFilename = 0;
      while (data.series[ProcessIndex].fields[1].labels?.processid === nodeProcess) {
        nodeThreads.push(data.series[ProcessIndex].fields[1].labels?.thread);
        nodeFileName.push(data.series[ProcessIndex + data.series.length / 2].fields[1].values);
        nodeFileName[ctrFilename] = nodeFileName[ctrFilename].buffer;
        ProcessIndex++;
        ctrFilename++;
      }

      //Node for Process
      if (!nodes.map((a: any) => a.name).includes(nodeProcess)) {
        nodes.push({
          index: nodeNumber,
          name: nodeProcess,
          r: 10,
          writtenBytes: data.series[ProcessIndex - nodeThreads.length].fields[1].values.get(0),
        });
        nodeNumber += 1;
        addingNodes = true;
      }

      //Get all Threads of the Process
      for (let i = 0; i < nodeThreads.length; i++) {
        //Expection for PID = TID
        if (!nodes.map((a: any) => a.name).includes(nodeThreads[i]) || nodeProcess === nodeThreads[i]) {
          nodes.push({
            index: nodeNumber,
            name: nodeThreads[i],
            r: 5,
            writtenBytes: data.series[ProcessIndex - nodeThreads.length + i].fields[1].values.get(0),
          });
          nodeNumber += 1;
          addingNodes = true;
        }
      }

      //Get all traced_filenames for each process
      for (let i = 0; i < nodeFileName.length; i++) {
        for (let j = 0; j < nodeFileName[i].length; j++) {
          //Add Writtenbytes per filename as aditional query? => Also needed for Drilldown Filename then
          nodes.push({ index: nodeNumber, name: nodeFileName[i][j], r: 5, writtenBytes: 0 });
          nodeNumber += 1;
          addingNodes = true;
        }
      }

      _callback();
    }

    //TODO Add actual written Bytes
    //Add Colour to the Nodes aswell or only for links? Node Colours depending on wether node is Process/Thread/Filename => Friday!
    function changeNodeSize(node: any, value: any) {
      let maxNodeSize = 30;
      let minNodeSize = 10;
      let maxWrittenBytes = 10000; //TODO rel Wert?
      node.writtenBytes += value; //TODO why sum?
      let nodeSize = node.writtenBytes / maxWrittenBytes;
      if (nodeSize >= 1) {
        node.r = maxNodeSize;
      } else if (nodeSize > 0.33) {
        node.r = nodeSize * maxNodeSize;
      } else {
        node.r = minNodeSize;
      }
    }

    function addLinks() {
      links.length = 0;
      //Assign Process
      let sourceT = nodes[0];
      changeNodeSize(nodes[0], nodes[0].writtenBytes);
      //Assign Threads
      for (let i = 0; i < 1 + nodeThreads.length; i++) {
        let targetT = nodes[i];
        changeNodeSize(nodes[targetT.index], nodes[i].writtenBytes);
        //TODO Change/delete Link Colour | not needed anymore? Colour by Filesystem? => Add legend?
        let link_color = '#003f5c';
        if (data.series[i].fields[1].labels?.functionname === 'fwrite') {
          link_color = '#ef5675';
        } else if (data.series[i].fields[1].labels?.functionname === 'write') {
          link_color = '#ffa600';
        } else if (data.series[i].fields[1].labels?.functionname === 'writev') {
          link_color = '#ff0000';
        }
        links.push({
          source: sourceT.index,
          target: targetT.index,
          color: link_color,
        });
      }
      //Assign Filesystem
      let j = 0;
      for (let i = 0; i < nodeThreads.length; i++) {
        let sourceFn = nodes[i + 1];
        for (let k = 0; k < nodeFileName[i].length; k++) {
          let targetFn = nodes[nodeThreads.length + 1 + j];
          //changeNodeSize
          j++;
          //LinkColour Filename
          links.push({
            source: sourceFn.index,
            target: targetFn.index,
            color: '#000fff',
          });
        }
      }
    }

    function forceSimulation(_callback: any) {
      let simulation = d3
        .forceSimulation(nodes)
        .force('link', d3.forceLink().links(links).distance(75).strength(4))
        .force('charge', d3.forceManyBody().strength(-8))
        .stop();
      simulation.tick(500);
      _callback();
    }

    function drawForceGraph() {
      const margin = { left: 30, top: 30, right: 30, bottom: 30 };
      const chartWidth = width - (margin.left + margin.right);
      const chartHeight = height - (margin.top + margin.bottom);
      let xBorder: [any, any] = d3.extent(nodes, function (d: any) {
        return +d.x;
      });
      let yBorder: [any, any] = d3.extent(nodes, function (d: any) {
        return +d.y;
      });
      const xScale = d3
        .scaleLinear()
        .domain([xBorder[0] - 50, xBorder[1] + 50])
        .range([0, chartWidth]);
      const yScale = d3
        .scaleLinear()
        .domain([yBorder[0] - 50, yBorder[1] + 50])
        .range([chartHeight, 0]);
      const svg = d3.select('#area');
      svg
        .selectAll('line')
        .data(links)
        .enter()
        .append('line')
        .attr('x1', function (d: any) {
          return xScale(d.source.x);
        })
        .attr('y1', function (d: any) {
          return yScale(d.source.y);
        })
        .attr('x2', function (d: any) {
          return xScale(d.target.x);
        })
        .attr('y2', function (d: any) {
          return yScale(d.target.y);
        })
        .attr('stroke', function (d: any) {
          return d.color;
        })
        .attr('stroke-width', 1.5);
      svg
        .selectAll('circle')
        .data(nodes)
        .enter()
        .append('circle')
        .attr('cx', function (d: any) {
          return xScale(d.x);
        })
        .attr('cy', function (d: any) {
          return yScale(d.y);
        })
        .attr('r', function (d: any) {
          return d.r;
        })
        .style('fill', '#888888');
    }

    function fixateNodes() {
      if (addingNodes === true) {
        for (let i = 0; i < nodes.length; i++) {
          if (nodes[i].x !== undefined && nodes[i].y !== undefined) {
            nodes[i].fx = nodes[i].x;
            nodes[i].fy = nodes[i].y;
          }
        }
      }
    }

    function runSimulation() {
      addNodes(() => addLinks());
      forceSimulation(() => drawForceGraph());
      fixateNodes();
      //Use sessionStorage? => IF PID changes clear sS
      sessionStorage.setItem('nodes', JSON.stringify(nodes));
    }

    if (data.series.length === 0) {
      d3.select('#area').selectAll('*').remove();
      d3.select('p').text('No data');
      sessionStorage.clear();
    } else {
      d3.select('p').text('');
      d3.select('#area').selectAll('*').remove();
      runSimulation();
    }
  }, [options, data, height, width]);
  return (
    <div className="App">
      <p></p>
      <svg id="area" height={height} width={width}></svg>
    </div>
  );
};
