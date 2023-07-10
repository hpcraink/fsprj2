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
      let nodeNumber = nodes.length;
      let attributeFileName: any = [];
      //find all related threads and filenames | data.series.length/2 for second query
      let ctrFilename = 0;
      let writeNodes: any[] = [];
      let readNodes: any[] = [];
      //Add Write
      if (options.UseWrite) {
        //Find Index of Process
        for (let i = 0; i < data.series.length / 2; i++) {
          if (data.series[i].fields[1].labels?.processid === nodeProcess) {
            ProcessIndex = i;
            break;
          }
        }
        while (data.series[ProcessIndex].fields[1].labels?.processid === nodeProcess) {
          //Add Filter
          if (options.UseFilterFilename) {
            let lFileName: any = [];
            let nFN: any = [];
            //Write
            lFileName.push(data.series[ProcessIndex + data.series.length / 2].fields[1].values);
            for (let i = 0; i < lFileName[0].buffer.length; i++) {
              if (lFileName[0].buffer[i].includes(options.FilterFilename)) {
                nFN.push(lFileName[0].buffer[i]);
              }
            }
            writeNodes.push(nFN);
          }
          //Get everything
          else {
            //Write
            let lWrite: any = [];
            lWrite.push(data.series[ProcessIndex + data.series.length / 2].fields[1].values);
            writeNodes[ctrFilename] = lWrite[0].buffer;
          }
          nodeThreads.push(data.series[ProcessIndex].fields[1].labels?.thread);
          ProcessIndex++;
          ctrFilename++;
        }
      }

      //Add Read
      if (options.UseRead) {
        //Find Index
        const startRead = data.series.findIndex((series) => series.refId === 'Read Bytes');
        for (let i = startRead; i < data.series.length / 2; i++) {
          if (data.series[i].fields[1].labels?.processid === nodeProcess) {
            ProcessIndex = i;
            break;
          }
        }
        while (data.series[ProcessIndex].fields[1].labels?.processid === nodeProcess) {
          //Add Filter
          if (options.UseFilterFilename) {
            let lFileName: any = [];
            let nFN: any = [];
            //Read
            lFileName.push(data.series[ProcessIndex + data.series.length / 2].fields[1].values);
            for (let i = 0; i < lFileName[0].buffer.length; i++) {
              if (lFileName[0].buffer[i].includes(options.FilterFilename)) {
                nFN.push(lFileName[0].buffer[i]);
              }
            }
            readNodes.push(nFN);
          }
          //Get everything
          else {
            //Read
            let lRead: any = [];
            lRead.push(data.series[ProcessIndex + data.series.length / 2].fields[1].values);
            readNodes[ctrFilename] = lRead[0].buffer;
          }
          //Check if Thread already there
          if (!nodeThreads.includes(data.series[ProcessIndex].fields[1].labels?.thread)) {
            nodeThreads.push(data.series[ProcessIndex].fields[1].labels?.thread);
          }
          ProcessIndex++;
          ctrFilename++;
        }
      }

      //Assign nodeFileName
      if (options.UseWrite && options.UseRead) {
        for (let i = 0; i < writeNodes.length; i++) {
          nodeFileName[i] = writeNodes[i].concat(readNodes[i]);
        }
        //Add Attribute read or write
        for (let i = 0; i < writeNodes.length; i++) {
          attributeFileName[i] = Array(writeNodes[i].length).fill('Write');
        }
        for (let i = 0; i < readNodes.length; i++) {
          attributeFileName[i] = attributeFileName[i].concat(Array(readNodes[i].length).fill('Read'));
        }
      } else if (options.UseWrite) {
        nodeFileName = writeNodes;
        for (let i = 0; i < writeNodes.length; i++) {
          attributeFileName[i] = Array(writeNodes[i].length).fill('Write');
        }
      } else if (options.UseRead) {
        nodeFileName = readNodes;
        for (let i = 0; i < readNodes.length; i++) {
          attributeFileName[i] = Array(readNodes[i].length).fill('Read');
        }
      }

      //Node for Process, skip existing nodes
      if (!nodes.map((a: any) => a.name).includes(nodeProcess)) {
        //TODO Node Colours gives mix of coulours or read/blue for more read/write
        nodes.push({
          index: nodeNumber,
          prefix: 'ProzessID: ',
          name: nodeProcess,
          affiliatedPrefix: 'Amount Read/Write: ',
          affiliated: 'Read: ', //TODO Add numbers
          affiliated2: 'Write: ',
          r: 10,
          writtenBytes: data.series[ProcessIndex - nodeThreads.length].fields[1].values.get(0),
          colour: '#888888',
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
          //Assign Colour
          let nodeColour: any;
          //Amount Read/Write
          let amountReadWrite: any = [0, 0];
          for (let j = 0; j < attributeFileName[i].length; j++) {
            if (attributeFileName[i][j] !== undefined) {
              if (attributeFileName[i][j] === 'Read') {
                amountReadWrite[0]++;
              } else if (attributeFileName[i][j] === 'Write') {
                amountReadWrite[1]++;
              }
            }
          }
          nodeColour = ColourMix(amountReadWrite);
          nodes.push({
            index: nodeNumber,
            prefix: 'ThreadID: ',
            name: nodeThreads[i],
            affiliatedPrefix: 'File Access: ',
            affiliated: 'Read: ' + amountReadWrite[0],
            affiliated2: 'Write: ' + amountReadWrite[1],
            r: 5,
            writtenBytes: data.series[ProcessIndex - nodeThreads.length + i].fields[1].values.get(0),
            colour: nodeColour,
          });
          nodeNumber += 1;
          addingNodes = true;
        }
      }

      //Get all traced_filenames for each process
      let IndexFilenamePerThread = nodeFileName.length + 1; //helpNodes to check the filenames of each thread instead of all filenames
      for (let i = 0; i < nodeFileName.length; i++) {
        for (let j = 0; j < nodeFileName[i].length; j++) {
          //TODO mark double filenames as write/read in affiliated (do with mpi test)
          if (
            !nodes
              .slice(IndexFilenamePerThread, IndexFilenamePerThread + nodeFileName[i].length)
              .map((a: any) => a.name)
              .includes(nodeFileName[i][j])
          ) {
            //Assign Colour
            let nodeColour: any;
            //Amount Read/Write
            let amountReadWrite: any = [0, 0];
            //for (let i = 0; i < lReadWrite.length; i++) { //use for later for more complex tests
            if (attributeFileName[i][j] === 'Read') {
              amountReadWrite[0]++;
            } else if (attributeFileName[i][j] === 'Write') {
              amountReadWrite[1]++;
            }
            //}
            nodeColour = ColourMix(amountReadWrite);
            //Push nodes
            nodes.push({
              index: nodeNumber,
              prefix: 'Filename: ',
              name: nodeFileName[i][j],
              affiliatedPrefix: 'File Access: ',
              affiliated: 'Read: ' + amountReadWrite[0],
              affiliated2: 'Write: ' + amountReadWrite[1],
              r: 5,
              writtenBytes: 0,
              colour: nodeColour,
            });
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
        //Link colour depending on wether last action was read or write? => Needed for Threads?
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
      let j = 1;
      for (let i = 0; i < nodeThreads.length; i++) {
        let sourceFn = nodes[i + 1];
        for (let k = 0; k < nodeFileName[i].length; k++) {
          let targetFn = nodes[nodeThreads.length + j];
          j++;
          //TODO
          //LinkColour Filename
          //Link colour depending on wether last action was read or write
          nodesLinks.push({
            source: sourceFn,
            target: targetFn.index - 1,
            colour: targetFn.colour,
          });
        }
      }
    }

    //Colourmix
    function ColourMix(lAmountReadWrite: any) {
      //Colour selection
      let colourNode: string;
      let RdWrtPercentage = 0;
      //Check for 0
      if (!(lAmountReadWrite[0] === 0 && lAmountReadWrite[1] === 0)) {
        if (lAmountReadWrite[1] === 0 && lAmountReadWrite[0] !== 0) {
          //onlyread
          RdWrtPercentage = 1;
        } else if (lAmountReadWrite[1] !== 0) {
          //only writes & mixed
          RdWrtPercentage = lAmountReadWrite[0] / lAmountReadWrite[1];
        }
        //build mixture of those Read = #323264, Write = #643232
        let redprop = (32 + 100 * (1 - RdWrtPercentage)).toString(16).substring(0, 2);
        let blueprop = (32 + 132 * RdWrtPercentage).toString(16).substring(0, 2);
        colourNode = '#' + redprop + '20' + blueprop;
        return colourNode;
      } else {
        return '#888888';
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
        .attr('fill', function (d: any) {
          return d.colour;
        })
        .on('mouseover', function (d) {
          d3.select(this).transition().duration(50).attr('opacity', '.85');
          backgroundTooltip.transition().duration(50).style('opacity', 1);
          textTooltip.transition().duration(50).style('opacity', 1);
          //Split Name if to long
          let lengthname: any,
            nameTT: any = [];
          if (d.name.length >= 50) {
            for (let i = 0; i <= d.name.length / 50; i++) {
              nameTT[i] = d.name.substring(i * 50, i * 50 + 50);
            }
            lengthname = (4 + nameTT.length) * 20 + 4; //d.affiliated.length + 2
          } else {
            lengthname = 5 * 20 + 4; //d.affiliated.length + 3
            nameTT[0] = d.name;
          }
          //Define width of backgrounTooltip
          //Check for longest Element
          let bckgrndTT = Math.max(d.prefix.length, nameTT[0].length, d.affiliatedPrefix.length, d.affiliated.length);
          //Position of Tooltip, default topright
          let xtspan = 0,
            xpos = this.cx.animVal.value,
            ypos = this.cy.animVal.value;
          if (xpos + 20 + bckgrndTT * 8 <= width && ypos - 40 >= 0) {
            backgroundTooltip.attr('x', xpos + 15).attr('y', ypos - 40);
            textTooltip.attr('x', xpos + 20).attr('y', ypos - 20);
            //move Tooltip up
            if (ypos - 40 + 3 * 20 + 4 >= height - 100) {
              //d.affiliated.length +
              backgroundTooltip.attr(
                'y',
                ypos - 40 - (ypos - 40 + (nameTT.length + 4) * 20 + 4 - height) //d.affiliated.length +
              );
              textTooltip.attr(
                'y',
                ypos - 20 - (ypos - 40 + (nameTT.length + 4) * 20 + 4 - height) //d.affiliated.length +
              );
            }
            xtspan = xpos + 20;
          }
          //Tooltip bottomleft
          else {
            backgroundTooltip.attr('x', xpos - 15 - bckgrndTT * 8).attr('y', ypos + 10);
            textTooltip.attr('x', xpos - 12 - bckgrndTT * 8).attr('y', yScale(d.y) + 30);
            //move Tooltip up
            if (ypos + 10 + 3 * 20 + 4 >= height - 100) {
              //d.affiliated.length +
              backgroundTooltip.attr(
                'y',
                ypos + 10 - (ypos + 10 + (nameTT.length + 4) * 20 + 4 - height) //d.affiliated.length +
              );
              textTooltip.attr(
                'y',
                ypos + 30 - (ypos + 10 + (nameTT.length + 4) * 20 + 4 - height) //d.affiliated.length +
              );
            }
            xtspan = xpos - 12 - bckgrndTT * 8;
          }
          backgroundTooltip
            .attr('width', bckgrndTT * 7 + 30)
            .attr('height', lengthname)
            .attr('rx', 3)
            .attr('fill', '#535353');
          textTooltip.attr('font-family', 'Arial').attr('font-size', 15).attr('fill', 'white');
          textTooltip.append('tspan').text(d.prefix).style('font-weight', 'bold');
          for (let i = 0; i < nameTT.length; i++) {
            textTooltip
              .append('tspan')
              .text(nameTT[i])
              .style('font-weight', 'normal')
              .attr('x', xtspan)
              .attr('dy', '1.2em');
          }
          textTooltip
            .append('tspan')
            .text(d.affiliatedPrefix)
            .style('font-weight', 'bold')
            .attr('x', xtspan)
            .attr('dy', '1.4em')
            .append('tspan')
            .text(d.affiliated)
            .attr('x', xtspan)
            .attr('dy', '1.2em')
            .style('font-weight', 'normal')
            .append('tspan')
            .text(d.affiliated2)
            .attr('x', xtspan)
            .attr('dy', '1.2em')
            .style('font-weight', 'normal');

          //Put Tooltip in the front
          backgroundTooltip.raise();
          textTooltip.raise();
        })
        .on('mouseout', function (d) {
          d3.select(this).transition().duration(50).attr('opacity', '1');
          backgroundTooltip.transition().duration(50).style('opacity', 0);
          textTooltip.transition().duration(50).style('opacity', 0);
          //remove old hover
          textTooltip.selectAll('tspan').remove();
          //Put Tooltip in the back
          backgroundTooltip.lower();
          textTooltip.lower();
        });
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

    //No old Data for testing purposes | TODO: Remove & Fix(?) for final tests!
    sessionStorage.clear();
  }, [options, data, height, width]);
  return (
    <div className="App">
      <p></p>
      <svg id="area" height={height} width={width}></svg>
    </div>
  );
};
