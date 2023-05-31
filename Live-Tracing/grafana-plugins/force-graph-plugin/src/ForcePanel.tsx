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
    //Get Nodeprocess from Plugin Settings
    let nodeProcess: any = options.ProcessID.toString();
    let nodeThreads: any = [];
    let nodeFileName: any = [];

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
        //Node Colours gives mix of coulours or read/blue for more read/write
        let link_color = '#003f5c';
        // if (data.series[i].fields[1].labels?.functionname === 'fwrite') {
        //   link_color = '#ef5675';
        // } else if (data.series[i].fields[1].labels?.functionname === 'write') {
        //   link_color = '#ffa600';
        // } else if (data.series[i].fields[1].labels?.functionname === 'writev') {
        //   link_color = '#ff0000';
        // }
        nodesLinks.push({
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
          //TODO: uncomment as soon as written bytes per filename in query is added:
          //changeNodeSize(targetFn.index) // might need to add nodeThreads.length
          j++;
          //TODO
          //LinkColour Filename
          //Link colour depending on wether last action was read or write
          nodesLinks.push({
            source: sourceFn,
            target: targetFn.index - 1,
            color: '#000fff',
          });
        }
      }
    }

    //Process and Threads
    //.force('center', d3.forceCenter(width / 2, height / 2)) 100, -30
    function forceSimulation(_callback: () => void) {
      // let simulationProcess = d3.forceSimulation(nodes[0]).force('center', d3.forceCenter(0, 0)).stop();
      // simulationProcess.tick(500);

      //        .force('center', d3.forceCenter(nodes[0].x + nodes[0].vx, nodes[0].y + nodes[0].vy))
      let simulationThread = d3
        .forceSimulation(nodes.slice(0, nodeThreads.length + 1))
        .force('charge', d3.forceManyBody().strength(-300))
        .force('link', d3.forceLink().links(nodesLinks.slice(0, nodeThreads.length)).distance(200).strength(4))
        .stop();
      simulationThread.tick(500);

      //TODO simplify
      let fileNameIndex = 0;
      for (let i = 0; i < nodeThreads.length; i++) {
        fileNameIndex += nodeFileName[i].length;
        const otherThreads = Array.from({ length: nodeThreads.length - 1 }, () => []);
        const lData = [nodes[i], ...otherThreads].concat(nodes.slice(nodeThreads.length + 1));
        const lLinks = nodesLinks.slice(nodeThreads.length, nodeThreads.length + fileNameIndex);
        let simulationFilename = d3
          .forceSimulation(lData)
          .force('charge', d3.forceManyBody().strength(-30))
          .force('link', d3.forceLink().links(lLinks).distance(75).strength(4))
          .stop();
        simulationFilename.tick(500);
      }
      _callback();
    }

    //Add scaling for r?

    function drawForceGraph() {
      const margin = { left: 20, top: 20, right: 20, bottom: 20 };
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
        .data(nodesLinks)
        .enter()
        .append('line')
        .attr('x1', function (d: any) {
          return xScale(d.source.x);
          //return d.source.x;
        })
        .attr('y1', function (d: any) {
          return yScale(d.source.y);
          //return d.source.y;
        })
        .attr('x2', function (d: any) {
          return xScale(d.target.x);
          //return d.target.x;
        })
        .attr('y2', function (d: any) {
          return yScale(d.target.y);
          //return d.target.y;
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
          //return d.x;
        })
        .attr('cy', function (d: any) {
          return yScale(d.y);
          //return d.y;
        })
        .attr('r', function (d: any) {
          return d.r;
        })
        .style('fill', '#888888');
    }

    //TODO Add function to draw mouseover/data
    function addMouseOver() {}

    //function without scaling
    // function drawForceGraph2() {
    //   const svg = d3.select('#area');
    //   svg
    //     .selectAll('line')
    //     .data(nodesLinks)
    //     .enter()
    //     .append('line')
    //     .attr('x1', function (d: any) {
    //       return d.source.x;
    //     })
    //     .attr('y1', function (d: any) {
    //       return d.source.y;
    //     })
    //     .attr('x2', function (d: any) {
    //       return d.target.x;
    //     })
    //     .attr('y2', function (d: any) {
    //       return d.target.y;
    //     })
    //     .attr('stroke', function (d: any) {
    //       return d.color;
    //     })
    //     .attr('stroke-width', 1.5);
    //   svg
    //     .selectAll('circle')
    //     .data(nodes)
    //     .enter()
    //     .append('circle')
    //     .attr('cx', function (d: any) {
    //       return d.x;
    //     })
    //     .attr('cy', function (d: any) {
    //       return d.y;
    //     })
    //     .attr('r', function (d: any) {
    //       return d.r;
    //     })
    //     .style('fill', '#888888');
    // }

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
      addMouseOver();
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
    //sessionStorage.clear();
  }, [options, data, height, width]);
  return (
    <div className="App">
      <p></p>
      <svg id="area" height={height} width={width}></svg>
    </div>
  );
};
