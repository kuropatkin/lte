/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <iostream>
#include <fstream>
#include <string>
#include "ns3/gnuplot.h"
#include <stdlib.h>

using namespace std;
using namespace ns3;

void Create2DPlotFile ()
{
  std::string fileNameWithNoExtension = "plot-2d";
  std::string graphicsFileName        = fileNameWithNoExtension + ".png";
  std::string plotFileName            = fileNameWithNoExtension + ".plt";
  std::string plotTitle               = "2-D Plot";
  std::string dataTitle               = "2-D Data";

  // Instantiate the plot and set its title.
  Gnuplot plot (graphicsFileName);
  plot.SetTitle (plotTitle);

  // Make the graphics file, which the plot file will create when it
  // is used with Gnuplot, be a PNG file.
  plot.SetTerminal ("png");

  // Set the labels for each axis.
  plot.SetLegend ("X Values", "Y Values");

  // Set the range for the x axis.
  plot.AppendExtra ("set xrange [-6:+6]");

  // Instantiate the dataset, set its title, and make the points be
  // plotted along with connecting lines.
  Gnuplot2dDataset dataset;
  dataset.SetTitle (dataTitle);
  dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  double x;
  double y;

  // Create the 2-D dataset.
  for (x = -5.0; x <= +5.0; x += 1.0)
    {
      // Calculate the 2-D curve
      //
      //            2
      //     y  =  x   .
      //
      y = x * x;

      // Add this point.
      dataset.Add (x, y);
    }

  // Add the dataset to the plot.
  plot.AddDataset (dataset);

  // Open the plot file.
  std::ofstream plotFile (plotFileName.c_str());

  // Write the plot file.
  plot.GenerateOutput (plotFile);

  // Close the plot file.
  plotFile.close ();
}

int
main (int argc, char *argv[])
{
  std::string fileNameWithNoExtension = "plot-2d";
  std::string graphicsFileName        = fileNameWithNoExtension + ".png";
  std::string plotFileName            = fileNameWithNoExtension + ".plt";
  std::string plotTitle               = "2-D Plot";
  std::string dataTitle               = "2-D Data";

  // Instantiate the plot and set its title.
  Gnuplot plot (graphicsFileName);
  plot.SetTitle (plotTitle);

  // Make the graphics file, which the plot file will create when it
  // is used with Gnuplot, be a PNG file.
  plot.SetTerminal ("png");

  // Set the labels for each axis.
  plot.SetLegend ("X Values", "Y Values");

  // Set the range for the x axis.
  plot.AppendExtra ("set xrange [0:10]");

  // Instantiate the dataset, set its title, and make the points be
  // plotted along with connecting lines.
  Gnuplot2dDataset dataset;
  dataset.SetTitle (dataTitle);
  dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  double x;
  double y;




  double *clientSendingTime = new double [5];
  double *serverSendingTime = new double [5];
  double *sendingTime = new double [5];
  int stringN = 0;
  string line;
  ifstream clientOutput ("ClientOutput.txt");
  if (clientOutput.is_open())
  {
    while ( getline (clientOutput,line) )
    {
      string strTime = line.substr(1, 5);
      double timeMark = atof(strTime.c_str()) / 10000;
      clientSendingTime[stringN] = timeMark;
      //cout << timeMark << '\n';
      stringN++;
    }
    clientOutput.close();
  }
  stringN = 0;
  ifstream serverOutput ("ServerOutput.txt");
  if (serverOutput.is_open())
  {
    while ( getline (serverOutput,line) )
    {
      string strTime = line.substr(1, 5);
      double timeMark = atof(strTime.c_str()) / 10000;
      serverSendingTime[stringN] = timeMark;
      //cout << timeMark << '\n';
      stringN++;
    }
    serverOutput.close();
  }
  
  for (int count = 0; count < 5; count++){
    sendingTime[count] = clientSendingTime[count] - serverSendingTime[count];
    cout << sendingTime[count] << "\n";
    
    x = clientSendingTime[count];
    y = 183 / (1024 * sendingTime[count]);

    // Add this point.
    dataset.Add (x, y);
  }
  // Add the dataset to the plot.
  plot.AddDataset (dataset);

  // Open the plot file.
  std::ofstream plotFile (plotFileName.c_str());

  // Write the plot file.
  plot.GenerateOutput (plotFile);

  // Close the plot file.
  plotFile.close ();
  delete [] clientSendingTime;
  delete [] serverSendingTime;
  delete [] sendingTime;
  return 0;
}


