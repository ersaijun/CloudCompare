#ifndef COMMAND_LINE_RASTER_HEADER
#define COMMAND_LINE_RASTER_HEADER

#include "ccCommandLineInterface.h"

//local
#include "ccRasterizeTool.h"

//Qt
#include <QString>

//qCC_db
#include <ccProgressDialog.h>
#include <ccVolumeCalcTool.h>

//shared commands
static const char COMMAND_GRID_VERT_DIR[]					= "VERT_DIR";
static const char COMMAND_GRID_STEP[]						= "GRID_STEP";
static const char COMMAND_GRID_OUTPUT_CLOUD[]				= "OUTPUT_CLOUD";
static const char COMMAND_GRID_OUTPUT_MESH[]				= "OUTPUT_MESH";
static const char COMMAND_GRID_OUTPUT_RASTER_Z[]			= "OUTPUT_RASTER_Z";
static const char COMMAND_GRID_OUTPUT_RASTER_RGB[]			= "OUTPUT_RASTER_RGB";

//Rasterize specific commands
static const char COMMAND_RASTERIZE[]						= "RASTERIZE";
static const char COMMAND_RASTER_CUSTOM_HEIGHT[]			= "CUSTOM_HEIGHT";
static const char COMMAND_RASTER_FILL_EMPTY_CELLS[]			= "EMPTY_FILL";
static const char COMMAND_RASTER_FILL_MIN_HEIGHT[]			= "MIN_H";
static const char COMMAND_RASTER_FILL_MAX_HEIGHT[]			= "MAX_H";
static const char COMMAND_RASTER_FILL_CUSTOM_HEIGHT[]		= "CUSTOM_H";
static const char COMMAND_RASTER_FILL_INTERPOLATE[]			= "INTERP";
static const char COMMAND_RASTER_PROJ_TYPE[]				= "PROJ";
static const char COMMAND_RASTER_SF_PROJ_TYPE[]				= "SF_PROJ";
static const char COMMAND_RASTER_PROJ_MIN[]					= "MIN";
static const char COMMAND_RASTER_PROJ_MAX[]					= "MAX";
static const char COMMAND_RASTER_PROJ_AVG[]					= "AVG";

//2.5D Volume calculation specific commands
static const char COMMAND_VOLUME[] = "VOLUME";
static const char COMMAND_VOLUME_GROUND_IS_FIRST[]			= "GROUND_IS_FIRST";
static const char COMMAND_VOLUME_CONST_HEIGHT[]				= "CONST_HEIGHT";

ccRasterGrid::ProjectionType GetProjectionType(QString option, ccCommandLineInterface& cmd)
{
	if (option == COMMAND_RASTER_PROJ_MIN)
	{
		return ccRasterGrid::PROJ_MINIMUM_VALUE;
	}
	else if (option == COMMAND_RASTER_PROJ_MAX)
	{
		return ccRasterGrid::PROJ_MAXIMUM_VALUE;
	}
	else if (option == COMMAND_RASTER_PROJ_AVG)
	{
		return ccRasterGrid::PROJ_AVERAGE_VALUE;
	}
	else
	{
		assert(false);
		cmd.warning(QString("Unknwon projection type: %1 (defaulting to 'average')").arg(option));
		return ccRasterGrid::PROJ_AVERAGE_VALUE;
	}
}

ccRasterGrid::EmptyCellFillOption GetEmptyCellFillingStrategy(QString option, ccCommandLineInterface& cmd)
{
	if (option == COMMAND_RASTER_FILL_MIN_HEIGHT)
	{
		return ccRasterGrid::FILL_MINIMUM_HEIGHT;
	}
	else if (option == COMMAND_RASTER_FILL_MAX_HEIGHT)
	{
		return ccRasterGrid::FILL_MAXIMUM_HEIGHT;
	}
	else if (option == COMMAND_RASTER_FILL_CUSTOM_HEIGHT)
	{
		return ccRasterGrid::FILL_CUSTOM_HEIGHT;
	}
	else if (option == COMMAND_RASTER_FILL_INTERPOLATE)
	{
		return ccRasterGrid::INTERPOLATE;
	}
	else
	{
		assert(false);
		cmd.warning(QString("Unknwon empty cell filling strategy: %1 (defaulting to 'leave empty')").arg(option));
		return ccRasterGrid::LEAVE_EMPTY;
	}
}

struct CommandRasterize : public ccCommandLineInterface::Command
{
	CommandRasterize() : ccCommandLineInterface::Command("Rasterize", COMMAND_RASTERIZE) {}

	virtual bool process(ccCommandLineInterface& cmd) override
	{
		cmd.print("[RASTERIZE]");

		//look for local options
		double gridStep = 0;
		bool outputCloud = false;
		bool outputRasterZ = false;
		bool outputRasterRGB = false;
		bool outputMesh = false;
		double customHeight = std::numeric_limits<double>::quiet_NaN();
		int vertDir = 2;
		ccRasterGrid::ProjectionType projectionType = ccRasterGrid::PROJ_AVERAGE_VALUE;
		ccRasterGrid::ProjectionType sfProjectionType = ccRasterGrid::PROJ_AVERAGE_VALUE;
		ccRasterGrid::EmptyCellFillOption emptyCellFillStrategy = ccRasterGrid::LEAVE_EMPTY;

		while (!cmd.arguments().empty())
		{
			QString argument = cmd.arguments().front();
			if (ccCommandLineInterface::IsCommand(argument, COMMAND_GRID_OUTPUT_CLOUD))
			{
				//local option confirmed, we can move on
				cmd.arguments().pop_front();

				outputCloud = true;
			}
			else if (ccCommandLineInterface::IsCommand(argument, COMMAND_GRID_OUTPUT_MESH))
			{
				//local option confirmed, we can move on
				cmd.arguments().pop_front();

				outputMesh = true;
			}
			else if (ccCommandLineInterface::IsCommand(argument, COMMAND_GRID_OUTPUT_RASTER_Z))
			{
				//local option confirmed, we can move on
				cmd.arguments().pop_front();

				outputRasterZ = true;
			}
			else if (ccCommandLineInterface::IsCommand(argument, COMMAND_GRID_OUTPUT_RASTER_RGB))
			{
				//local option confirmed, we can move on
				cmd.arguments().pop_front();

				outputRasterRGB = true;
			}
			else if (ccCommandLineInterface::IsCommand(argument, COMMAND_GRID_STEP))
			{
				//local option confirmed, we can move on
				cmd.arguments().pop_front();

				bool ok;
				gridStep = cmd.arguments().takeFirst().toDouble(&ok);
				if (!ok || gridStep <= 0)
				{
					return cmd.error(QString("Invalid grid step value! (after %1)").arg(COMMAND_GRID_STEP));
				}
			}
			else if (ccCommandLineInterface::IsCommand(argument, COMMAND_RASTER_CUSTOM_HEIGHT))
			{
				//local option confirmed, we can move on
				cmd.arguments().pop_front();

				bool ok;
				customHeight = cmd.arguments().takeFirst().toDouble(&ok);
				if (!ok)
				{
					return cmd.error(QString("Invalid custom height value! (after %1)").arg(COMMAND_RASTER_CUSTOM_HEIGHT));
				}
			}
			else if (ccCommandLineInterface::IsCommand(argument, COMMAND_GRID_VERT_DIR))
			{
				//local option confirmed, we can move on
				cmd.arguments().pop_front();

				bool ok;
				vertDir = cmd.arguments().takeFirst().toInt(&ok);
				if (!ok || vertDir < 0 || vertDir > 2)
				{
					return cmd.error(QString("Invalid vert. direction! (after %1)").arg(COMMAND_GRID_VERT_DIR));
				}
			}
			else if (ccCommandLineInterface::IsCommand(argument, COMMAND_RASTER_FILL_EMPTY_CELLS))
			{
				//local option confirmed, we can move on
				cmd.arguments().pop_front();

				emptyCellFillStrategy = GetEmptyCellFillingStrategy(cmd.arguments().takeFirst().toUpper(), cmd);
			}
			else if (ccCommandLineInterface::IsCommand(argument, COMMAND_RASTER_PROJ_TYPE))
			{
				//local option confirmed, we can move on
				cmd.arguments().pop_front();

				projectionType = GetProjectionType(cmd.arguments().takeFirst().toUpper(), cmd);
			}
			else if (ccCommandLineInterface::IsCommand(argument, COMMAND_RASTER_SF_PROJ_TYPE))
			{
				//local option confirmed, we can move on
				cmd.arguments().pop_front();

				sfProjectionType = GetProjectionType(cmd.arguments().takeFirst().toUpper(), cmd);
			}
		}

		if (gridStep == 0)
		{
			return cmd.error(QString("Grid step value not defined (use %1)").arg(COMMAND_GRID_STEP));
		}

		if (emptyCellFillStrategy == ccRasterGrid::FILL_CUSTOM_HEIGHT && isnan(customHeight))
		{
			cmd.error("[Rasterize] The filling stragety is set to 'fill with custom height' but not custom height was defined...");
			emptyCellFillStrategy = ccRasterGrid::LEAVE_EMPTY;
		}

		if (!outputCloud && !outputMesh && !outputRasterZ && !outputRasterRGB)
		{
			//if no export target is specified, we chose the cloud by default
			outputCloud = true;
		}

		//we'll get the first two clouds
		for (const CLCloudDesc& cloudDesc : cmd.clouds())
		{
			if (!cloudDesc.pc)
			{
				assert(false);
				continue;
			}
			ccBBox gridBBox = cloudDesc.pc->getOwnBB();

			//compute the grid size
			unsigned gridWidth = 0, gridHeight = 0;
			if (!ccRasterGrid::ComputeGridSize(vertDir, gridBBox, gridStep, gridWidth, gridHeight))
			{
				return cmd.error("Failed to compute the grid dimensions (check input cloud(s) bounding-box)");
			}

			ccRasterGrid grid;
			{
				//memory allocation
				CCVector3d minCorner = CCVector3d::fromArray(gridBBox.minCorner().u);
				if (!grid.init(gridWidth, gridHeight, gridStep, minCorner))
				{
					//not enough memory
					return cmd.error("Not enough memory");
				}

				//progress dialog
				QScopedPointer<ccProgressDialog> pDlg(0);
				if (!cmd.silentMode())
				{
					pDlg.reset(new ccProgressDialog(true, cmd.widgetParent()));
				}

				if (grid.fillWith(cloudDesc.pc,
					vertDir,
					projectionType,
					emptyCellFillStrategy == ccRasterGrid::INTERPOLATE,
					sfProjectionType,
					pDlg.data()))
				{
					grid.fillEmptyCells(emptyCellFillStrategy, customHeight);
					cmd.print(QString("[Rasterize] Raster grid: size: %1 x %2 / heights: [%3 ; %4]").arg(grid.width).arg(grid.height).arg(grid.minHeight).arg(grid.maxHeight));
				}
				else
				{
					return cmd.error("Rasterize process failed");
				}
			}

			//generate the result entity (cloud by default)
			if (outputCloud || outputMesh)
			{
				ccPointCloud* rasterCloud = ccVolumeCalcTool::ConvertGridToCloud(grid, gridBBox, vertDir, true);
				if (!rasterCloud)
				{
					return cmd.error("Failed to output the raster grid as a cloud");
				}

				ccMesh* rasterMesh = 0;
				if (outputMesh)
				{
					char errorStr[1024];
					CCLib::GenericIndexedMesh* baseMesh = CCLib::PointProjectionTools::computeTriangulation
						(
						rasterCloud,
						DELAUNAY_2D_AXIS_ALIGNED,
						0,
						vertDir,
						errorStr
						);
					if (baseMesh)
					{
						rasterMesh = new ccMesh(baseMesh, rasterCloud);
						delete baseMesh;
						baseMesh = 0;
					}

					if (rasterMesh)
					{
						rasterCloud->setEnabled(false);
						rasterCloud->setVisible(true);
						rasterMesh->addChild(rasterCloud);
						rasterMesh->setName(rasterCloud->getName());
						rasterCloud->setName("vertices");
						rasterMesh->showSF(rasterCloud->sfShown());
						rasterMesh->showColors(rasterCloud->colorsShown());

						cmd.print(QString("[Rasterize] Mesh '%1' successfully generated").arg(rasterMesh->getName()));
					}
					else
					{
						delete rasterCloud;
						return cmd.error(QString("[Rasterize] Failed to create output mesh ('%1')").arg(errorStr));
					}
				}

				if (rasterCloud)
				{
					if (outputCloud)
					{
						CLCloudDesc cloudDesc;
						cloudDesc.pc = rasterCloud;
						cloudDesc.basename = cloudDesc.basename;
						cloudDesc.path = cloudDesc.path;
						QString outputFilename;
						QString errorStr = cmd.exportEntity(cloudDesc, "RASTER", &outputFilename);
						if (!errorStr.isEmpty())
						{
							cmd.warning(errorStr);
						}
					}
					delete rasterCloud;
					rasterCloud = 0;
				}

				if (rasterMesh)
				{
					assert(outputMesh);
					CLMeshDesc meshDesc;
					meshDesc.mesh = rasterMesh;
					meshDesc.basename = cloudDesc.basename;
					meshDesc.path = cloudDesc.path;
					QString outputFilename;
					QString errorStr = cmd.exportEntity(meshDesc, "RASTER", &outputFilename);
					delete rasterMesh;
					rasterMesh = 0;
					if (!errorStr.isEmpty())
						cmd.warning(errorStr);
				}
			}

			if (outputRasterZ || outputRasterRGB)
			{
				ccRasterizeTool::ExportBands bands;
				{
					bands.height = outputRasterZ;
					bands.rgb = outputRasterRGB; //not a good idea to mix RGB and height values!
					bands.allSFs = outputRasterZ;
				}
				QString exportFilename = cmd.getExportFilename(cloudDesc, "RASTER", 0, 0, false, cmd.addTimestamp());
				if (exportFilename.isEmpty())
				{
					exportFilename = "raster.geotiff";
				}

				ccRasterizeTool::ExportGeoTiff(exportFilename, bands, emptyCellFillStrategy, grid, gridBBox, vertDir, customHeight, cloudDesc.pc);
			}
		}

		return true;
	}
};

struct CommandVolume25D : public ccCommandLineInterface::Command
{
	CommandVolume25D() : ccCommandLineInterface::Command("2.5D Volume Calculation", COMMAND_VOLUME) {}

	virtual bool process(ccCommandLineInterface& cmd) override
	{
		cmd.print("[2.5D VOLUME]");

		//look for local options
		bool groundIsFirst = false;
		double gridStep = 0;
		double constHeight = std::numeric_limits<double>::quiet_NaN();
		bool outputMesh = false;
		int vertDir = 2;

		while (!cmd.arguments().empty())
		{
			QString argument = cmd.arguments().front();
			if (ccCommandLineInterface::IsCommand(argument, COMMAND_VOLUME_GROUND_IS_FIRST))
			{
				//local option confirmed, we can move on
				cmd.arguments().pop_front();

				groundIsFirst = true;
			}
			else if (ccCommandLineInterface::IsCommand(argument, COMMAND_GRID_OUTPUT_MESH))
			{
				//local option confirmed, we can move on
				cmd.arguments().pop_front();

				outputMesh = true;
			}
			else if (ccCommandLineInterface::IsCommand(argument, COMMAND_GRID_STEP))
			{
				//local option confirmed, we can move on
				cmd.arguments().pop_front();

				bool ok;
				gridStep = cmd.arguments().takeFirst().toDouble(&ok);
				if (!ok || gridStep <= 0)
				{
					return cmd.error(QString("Invalid grid step value! (after %1)").arg(COMMAND_GRID_STEP));
				}
			}
			else if (ccCommandLineInterface::IsCommand(argument, COMMAND_VOLUME_CONST_HEIGHT))
			{
				//local option confirmed, we can move on
				cmd.arguments().pop_front();

				bool ok;
				constHeight = cmd.arguments().takeFirst().toDouble(&ok);
				if (!ok)
				{
					return cmd.error(QString("Invalid const. height value! (after %1)").arg(COMMAND_VOLUME_CONST_HEIGHT));
				}
			}
			else if (ccCommandLineInterface::IsCommand(argument, COMMAND_GRID_VERT_DIR))
			{
				//local option confirmed, we can move on
				cmd.arguments().pop_front();

				bool ok;
				vertDir = cmd.arguments().takeFirst().toInt(&ok);
				if (!ok || vertDir < 0 || vertDir > 2)
				{
					return cmd.error(QString("Invalid vert. direction! (after %1)").arg(COMMAND_GRID_VERT_DIR));
				}
			}
		}

		if (gridStep == 0)
		{
			return cmd.error(QString("Grid step value not defined (use %1)").arg(COMMAND_GRID_STEP));
		}

		//we'll get the first two clouds
		CLCloudDesc *ground = 0, *ceil = 0;
		{
			CLCloudDesc* clouds[2] = { 0, 0 };
			int index = 0;
			if (!cmd.clouds().empty())
			{
				clouds[index++] = &cmd.clouds()[0];
				if (std::isnan(constHeight) && cmd.clouds().size() > 1)
				{
					clouds[index++] = &cmd.clouds()[1];
				}
			}

			int expectedCount = std::isnan(constHeight) ? 2 : 1;
			if (index != expectedCount)
			{
				return cmd.error(QString("Not enough loaded entities (%1 found, %2 expected)").arg(index).arg(expectedCount));
			}

			if (index == 2 && groundIsFirst)
			{
				//put them in the right order (ground then ceil)
				std::swap(clouds[0], clouds[1]);
			}

			ceil = clouds[0];
			ground = clouds[1];
		}

		ccBBox gridBBox = ceil ? ceil->pc->getOwnBB() : ccBBox();
		if (ground)
		{
			gridBBox += ground->pc->getOwnBB();
		}

		//compute the grid size
		unsigned gridWidth = 0, gridHeight = 0;
		if (!ccRasterGrid::ComputeGridSize(vertDir, gridBBox, gridStep, gridWidth, gridHeight))
		{
			return cmd.error("Failed to compute the grid dimensions (check input cloud(s) bounding-box)");
		}

		ccRasterGrid grid;
		ccVolumeCalcTool::ReportInfo reportInfo;
		if (ccVolumeCalcTool::ComputeVolume(grid,
			ground ? ground->pc : 0,
			ceil ? ceil->pc : 0,
			gridBBox,
			vertDir,
			gridStep,
			gridWidth,
			gridHeight,
			ccRasterGrid::PROJ_AVERAGE_VALUE,
			ccRasterGrid::LEAVE_EMPTY,
			reportInfo,
			constHeight,
			constHeight,
			cmd.silentMode() ? 0 : cmd.widgetParent()))
		{
			CLCloudDesc* desc = ceil ? ceil : ground;
			assert(desc);

			//save repot in a separate text file
			{
				QString txtFilename = QString("%1/%2").arg(desc->path).arg("VolumeCalculationReport");
				if (cmd.addTimestamp())
					txtFilename += QString("_%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh'h'mm"));
				txtFilename += QString(".txt");

				QFile txtFile(txtFilename);
				txtFile.open(QIODevice::WriteOnly | QIODevice::Text);
				QTextStream txtStream(&txtFile);
				txtStream << reportInfo.toText() << endl;
				txtFile.close();
			}

			//generate the result entity (cloud by default)
			{
				ccPointCloud* rasterCloud = ccVolumeCalcTool::ConvertGridToCloud(grid, gridBBox, vertDir, true);
				if (!rasterCloud)
				{
					return cmd.error("Failed to output the volume grid");
				}
				if (rasterCloud->hasScalarFields())
				{
					//convert SF to RGB
					//rasterCloud->setCurrentDisplayedScalarField(0);
					rasterCloud->setRGBColorWithCurrentScalarField(false);
					rasterCloud->showColors(true);
				}

				ccMesh* rasterMesh = 0;
				if (outputMesh)
				{
					char errorStr[1024];
					CCLib::GenericIndexedMesh* baseMesh = CCLib::PointProjectionTools::computeTriangulation(rasterCloud,
						DELAUNAY_2D_AXIS_ALIGNED,
						0,
						vertDir,
						errorStr);

					if (baseMesh)
					{
						rasterMesh = new ccMesh(baseMesh, rasterCloud);
						delete baseMesh;
						baseMesh = 0;
					}

					if (rasterMesh)
					{
						rasterCloud->setEnabled(false);
						rasterCloud->setVisible(true);
						rasterMesh->addChild(rasterCloud);
						rasterMesh->setName(rasterCloud->getName());
						rasterCloud->setName("vertices");
						rasterMesh->showSF(rasterCloud->sfShown());
						rasterMesh->showColors(rasterCloud->colorsShown());

						cmd.print(QString("[Volume] Mesh '%1' successfully generated").arg(rasterMesh->getName()));
					}
					else
					{
						delete rasterCloud;
						return cmd.error(QString("[Voume] Failed to create output mesh ('%1')").arg(errorStr));
					}
				}

				CLEntityDesc* outputDesc = 0;
				if (rasterMesh)
				{
					CLMeshDesc meshDesc;
					meshDesc.mesh = rasterMesh;
					meshDesc.basename = desc->basename;
					meshDesc.path = desc->path;
					cmd.meshes().push_back(meshDesc);
					outputDesc = &cmd.meshes().back();
				}
				else
				{
					CLCloudDesc cloudDesc;
					cloudDesc.pc = rasterCloud;
					cloudDesc.basename = desc->basename;
					cloudDesc.path = desc->path;
					cmd.clouds().push_back(cloudDesc);
					outputDesc = &cmd.clouds().back();
				}

				//save result as a PLY file
				if (outputDesc)
				{
					QString outputFilename;
					QString errorStr = cmd.exportEntity(*outputDesc, "HEIGHT_DIFFERENCE", &outputFilename);
					if (!errorStr.isEmpty())
						cmd.warning(errorStr);
				}
			}
		}
		else
		{
			return cmd.error("Failed to compte the volume");
		}

		return true;
	}
};

#endif //COMMAND_LINE_RASTER_HEADER