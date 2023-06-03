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
    let nodesLinks: any = [];
    let ProcessIndex = 0;
    let nodeProcess: any;
    let nodeThreads: any = [];
    let nodeFileName: any = [];
    //Check if ThreadMap is already built
    let selectNode = document.querySelector('#ThreadMapMain');

    if (
      selectNode === null ||
      d3.select('#ThreadMapMain').select('text[class="Forcegraph"').attr('ProcessID') === 'select'
    ) {
      d3.select('#area').selectAll('*').remove();
      d3.select('p').text('Select process in ThreadMap and refresh dashboard');
      return;
    } else {
      //Get Process from ThreadMap
      nodeProcess = d3.select('#ThreadMapMain').select('text[class="Forcegraph"').attr('ProcessID');
    }

    //Get old Nodes
    if (JSON.parse(sessionStorage.getItem('nodes')!) !== null) {
      nodes = JSON.parse(sessionStorage.getItem('nodes')!);
      //If PID changes, dont use old nodes => clear sessionStorage
      if (!(nodes[0].name === nodeProcess)) {
        nodes = [];
        sessionStorage.clear();
        console.log('NODES', nodes, 'Links', nodesLinks, 'Storage', sessionStorage);
      }
    }

    function addNodes(_callback: any) {
      //TODO Node Colours gives mix of coulours or read/blue for more read/write
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

      //Node for Process, skip existing nodes
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
        //Check for existing nodes w/o ProcessNode, skip existing nodes
        if (
          !nodes
            .slice(1)
            .map((a: any) => a.name)
            .includes(nodeThreads[i])
        ) {
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
      let IndexFilenamePerThread = nodeFileName.length + 1; //helpNodes to check the filenames of each thread instead of all filenames
      for (let i = 0; i < nodeFileName.length; i++) {
        for (let j = 0; j < nodeFileName[i].length; j++) {
          //TODO Add writtenBytes per filename as aditional query? => Also needed for Drilldown Filename then
          //skip existing nodes
          if (
            !nodes
              .slice(IndexFilenamePerThread, IndexFilenamePerThread + nodeFileName[i].length)
              .map((a: any) => a.name)
              .includes(nodeFileName[i][j])
          ) {
            nodes.push({ index: nodeNumber, name: nodeFileName[i][j], r: 5, writtenBytes: 0 });
            nodeNumber += 1;
            addingNodes = true;
          }
        }
        IndexFilenamePerThread += nodeFileName[i].length;
      }

      _callback();
    }

    function changeNodeSize(node: any) {
      const maxNodeSize = 20;
      const minNodeSize = 8;
      let maxWrittenBytes = 0;
      if (maxWrittenBytes === 0 || maxWrittenBytes < node.writtenBytes) {
        let nodeBytes: any = [];
        for (let i = 0; i < nodes.length; i++) {
          nodeBytes[i] = nodes[i].writtenBytes;
        }
        maxWrittenBytes = Math.max(...nodeBytes);
      }

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
      nodesLinks.length = 0;
      //Assign Process
      let sourceT = nodes[0];
      changeNodeSize(nodes[0]);
      //Assign Threads
      for (let i = 1; i < 1 + nodeThreads.length; i++) {
        let targetT = nodes[i];
        changeNodeSize(nodes[targetT.index]);
        //TODO
        //Add Colour to the Nodes aswell and for links
        //Link colour depending on wether last action was read or write
        let link_colour = '#003f5c';
        // if (data.series[i].fields[1].labels?.functionname === 'fwrite') {
        //   link_colour = '#ef5675';
        // } else if (data.series[i].fields[1].labels?.functionname === 'write') {
        //   link_colour = '#ffa600';
        // } else if (data.series[i].fields[1].labels?.functionname === 'writev') {
        //   link_colour = '#ff0000';
        // }
        nodesLinks.push({
          source: sourceT.index,
          target: targetT.index,
          colour: link_colour,
        });
      }
      //Assign Filesystem
      let j = 0;
      for (let i = 0; i < nodeThreads.length; i++) {
        let sourceFn = nodes[i + 1];
        for (let k = 0; k < nodeFileName[i].length; k++) {
          let targetFn = nodes[nodeThreads.length + 1 + j];
          //TODO: uncomment as soon as written bytes per filename in query is added:
          //changeNodeSize(targetFn.index) // might need to add nodeThreads.length
          j++;
          //TODO
          //LinkColour Filename
          //Link colour depending on wether last action was read or write
          nodesLinks.push({
            source: sourceFn,
            target: targetFn.index - 1,
            colour: '#000fff',
          });
        }
      }
    }

    //Process and Threads
    function forceSimulation(_callback: () => void) {
      //Draw Threads and Process
      let simulationThread = d3
        .forceSimulation(nodes.slice(0, nodeThreads.length + 1))
        .force('charge', d3.forceManyBody().strength(-200))
        .force('link', d3.forceLink().links(nodesLinks.slice(0, nodeThreads.length)).distance(100).strength(4))
        .stop();
      simulationThread.tick(500);

      //Draw Filenames
      const otherThreads = Array.from({ length: nodeThreads.length - 1 }, () => []);
      const lData = [nodes.slice(0, nodeThreads.length + 1), ...otherThreads].concat(
        nodes.slice(nodeThreads.length + 1)
      );
      const lLinks = nodesLinks.slice(nodeThreads.length);
      let simulationFilename = d3
        .forceSimulation(lData)
        .force('charge', d3.forceManyBody().strength(-40))
        .force('link', d3.forceLink().links(lLinks).distance(50).strength(4))
        .stop();
      simulationFilename.tick(500);

      _callback();
    }

    function drawForceGraph() {
      const margin = { left: 20, top: 10, right: 20, bottom: 10 };
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
        .domain([xBorder[0] - 30, xBorder[1] + 30])
        .range([0, chartWidth]);
      const yScale = d3
        .scaleLinear()
        .domain([yBorder[0] - 15, yBorder[1] + 15])
        .range([chartHeight, 0]);
      const svg = d3.select('#area');
      const backgroundTooltip = d3
        .select('#area')
        .append('rect')
        .attr('class', 'tooltip-background-fg')
        .style('opacity', 0);
      const textTooltip = d3.select('#area').append('text').attr('class', 'tooltip-text-fg').style('opacity', 0);

      //Draw Links
      svg
        .selectAll('line')
        .data(nodesLinks)
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
          return d.colour;
        })
        .attr('stroke-width', 1.5);
      //Draw Nodes
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
        .style('fill', '#888888')
        .on('mouseover', function (d) {
          d3.select(this).transition().duration(50).attr('opacity', '.85');
          backgroundTooltip.transition().duration(50).style('opacity', 1);
          textTooltip.transition().duration(50).style('opacity', 1);
          backgroundTooltip
            .attr('x', xScale(d.x) + 15)
            .attr('y', yScale(d.y) - 40)
            .attr('width', d.name.length * 8)
            .attr('height', '25')
            .attr('rx', '3')
            .attr('fill', '#888888'); //replace with d.colour when colour is implemented
          //TODO Add textwrap (with tspan?)
          textTooltip
            .attr('x', xScale(d.x) + 20)
            .attr('y', yScale(d.y) - 20)
            .attr('font-family', 'Arial')
            .attr('font-size', '15')
            .attr('fill', 'white')
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

    //Can be removed if session storage is used?
    function fixateNodes() {
      if (addingNodes === true) {
        // for (let i = 0; i < nodes.length; i++) {
        //   if (nodes[i].x !== undefined && nodes[i].y !== undefined) {
        //     nodes[i].fx = nodes[i].x;
        //     nodes[i].fy = nodes[i].y;
        //   }
        // }
      }
    }

    function runSimulation() {
      addNodes(() => addLinks());
      forceSimulation(drawForceGraph);
      fixateNodes();
      //Add Links to sS?
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

    //No old Data for testing purposes | Remove for final tests!
    sessionStorage.clear();
  }, [options, data, height, width]);
  return (
    <div className="App">
      <p></p>
      <svg id="area" height={height} width={width}></svg>
    </div>
  );
};
