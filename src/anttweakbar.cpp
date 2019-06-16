#include <ardev/ardevconfig.h>
#include <ardev/anttweakbar.h>

#ifdef HAVE_PLAYER
#include <ardev/player.h>
#endif

#ifdef HAVE_ANTTWEAKBAR

// sets up the intial tweak bar settings and default windows
TweakBar::TweakBar()
{
	TwInit(TW_OPENGL,NULL);

	b_displayList = false;
	b_environments = false;
	b_data = false;

	mainbar = TwNewBar("Main");
	displaybar = TwNewBar("Display List");
	envbar = TwNewBar("Environments");
	databar = TwNewBar("Data");
	selectbar = TwNewBar("Select");

    TwDefine(" Main label='Main' help='Main window' ");
    TwDefine(" Main  position='5 75' size='200 150' alwaysbottom=true ");
    TwAddButton(mainbar,"bDisplayList",TweakBar::toggleDisplayList,this," label='Display List' ");
    TwAddButton(mainbar,"bEnvironments",TweakBar::toggleEnvironments,this," label='Environments' ");
    TwAddButton(mainbar,"bData",TweakBar::toggleData,this," label='Data' ");
    TwAddSeparator(mainbar,NULL,NULL);
#ifdef HAVE_PLAYER
    TwAddButton(mainbar,"bDisconnect",PlayerDisconnect,NULL," label='Disconnect from Player' ");
	TwAddVarCB(mainbar, "auto" ,TW_TYPE_BOOLCPP,TweakBar::SetAutoConnectTW,TweakBar::GetAutoConnectTW,this, " label='Auto Connect' " );
    TwAddSeparator(mainbar,NULL,NULL);
#endif
    TwAddButton(mainbar,"bExit",Exit,this," label='Exit' ");

    TwDefine(" 'Display List' label='Display List' help='Window displaying all render objects' iconifiable=false ");
    TwDefine(" 'Display List' position='208 75' size='210 600' ");
    TwDefine(" 'Display List' visible=false ");

    TwDefine(" 'Environments' label='Environments' help='Window displaying all render objects' iconifiable=false ");
    TwDefine(" 'Environments' position='5 230' size='200 300' ");
    TwDefine(" 'Environments' visible=false ");

    TwAddButton(envbar,"EnvEmpty",NULL,NULL," group='Views' ");
    TwAddSeparator(envbar,NULL,NULL);
    TwAddButton(envbar,"bNext",TweakBar::nextEnvironment,this," label='Next' ");
    TwAddButton(envbar,"bPrev",TweakBar::previousEnvironment,this," label='Previous' ");

    TwDefine(" 'Data' label='Data' help='Window displaying data from various objects' iconifiable=false ");
    TwDefine(" 'Data' position='5 533' size='200 300' ");
    TwDefine(" 'Data' visible=false ");

    TwDefine(" 'Select' label='Select' help='Window displaying the selected object' iconifiable=false ");
    TwDefine(" 'Select' position='421 75' size='200 150' ");
    TwDefine(" 'Select' visible=false ");



    //TwDefine(" GLOBAL refresh=0.5 color='0 128 255' alpha=220 text=dark ");
}

// removes all tweak bars and shuts down
TweakBar::~TweakBar()
{
	TwRemoveAllVars(mainbar);
	TwDeleteBar(mainbar);
	TwRemoveAllVars(displaybar);
	TwDeleteBar(displaybar);
	TwRemoveAllVars(envbar);
	TwDeleteBar(envbar);
	TwRemoveAllVars(databar);
	TwDeleteBar(databar);
	TwRemoveAllVars(selectbar);
	TwDeleteBar(selectbar);
	TwTerminate();
}

// creates the display list window and controls
void TweakBar::generateDisplayMenu(RenderList &List)
{
	TwRemoveAllVars(displaybar);
    TwAddButton(displaybar,"lObjects",NULL,NULL," label='Render Objects' ");
    TwAddButton(displaybar,"bClear",TweakBar::clear,this," label='Clear' ");
    TwAddSeparator(displaybar,NULL,NULL);

	int i =0;
	for (RenderList::iterator itr = List.begin(); itr != List.end(); ++itr, i++)
	{
		char args[100];
		char uid[100];
		char close[100];

		sprintf(uid,"render%d",i);
		sprintf(args," group='%p'  true='Enabled' false='Disabled' label='Render' ",itr->Rend);
		TwAddVarCB(displaybar, strdup(uid) ,TW_TYPE_BOOLCPP,SetEnabledTW,GetEnabledTW,itr->Rend, strdup(args) );

		if(itr->Rend->colourConfig)
		{
			char alpha[10];
			sprintf(uid,"render%d",++i);
			sprintf(alpha,"%salpha",itr->Rend->alphaConfig?"":"no");
			sprintf(args," group='%p' %s label='Colour' ",itr->Rend,alpha);
			TwAddVarCB(displaybar, strdup(uid) ,TW_TYPE_COLOR32,SetColourTW,GetColourTW,itr->Rend, strdup(args) );
		}
		itr->Rend->DisplaySetup(this, itr->Name);

		sprintf(close," 'Display List'/'%p' close ",itr->Rend);
		TwDefine(close); // Closes the menu
	}

	for (RenderList::iterator iter = List.begin(); iter != List.end(); iter++)
	{
		// Do first level groups
		if (iter->Pname && iter->Name)
		{
			char p[100];
			sprintf(p," 'Display List'/'%p' group='%p' label='%s' ",iter->Rend,iter->Pos,iter->Name);
			TwDefine(p);
		}
		// Do other group levels
		PositionObject* pos = iter->Pos;
		while (pos)
		{
			if (pos->Next)
			{
				char p[100];
				sprintf(p," 'Display List'/'%p' group='%p' label='%s' ",pos,pos->Next,pos->GetName());
				TwDefine(p);
			}
			else // Top level, just rename it
			{
				char p[100];
				sprintf(p," 'Display List'/'%p' label='%s' ",pos,pos->GetName());
				TwDefine(p);
			}

			pos = pos->Next;
		}
	}
}

// toogles the display list window
void TW_CALL TweakBar::toggleDisplayList(void* data)
{
	TweakBar* tweakBar = (TweakBar*)data;
	if(tweakBar->b_displayList)
		TwDefine(" 'Display List' visible=false ");
	else
		TwDefine(" 'Display List' visible=true ");

	tweakBar->b_displayList=!tweakBar->b_displayList;
}

// toggles the environments window
void TW_CALL TweakBar::toggleEnvironments(void* data)
{
	TweakBar* tweakBar = (TweakBar*)data;
	if(tweakBar->b_environments)
		TwDefine(" 'Environments' visible=false ");
	else
		TwDefine(" 'Environments' visible=true ");

	tweakBar->b_environments=!tweakBar->b_environments;
}

// toggles the environments window
void TW_CALL TweakBar::toggleData(void* data)
{
	TweakBar* tweakBar = (TweakBar*)data;
	if(tweakBar->b_data)
		TwDefine(" 'Data' visible=false ");
	else
		TwDefine(" 'Data' visible=true ");

	tweakBar->b_data=!tweakBar->b_data;
}

// callback for next env button
void TW_CALL TweakBar::nextEnvironment(void*)
{
	ARDev::PublishEvent(new EnvironmentEventObject(EOE_RelativeEnvironment,1));
}

// callback for prev env button
void TW_CALL TweakBar::previousEnvironment(void*)
{
	ARDev::PublishEvent(new EnvironmentEventObject(EOE_RelativeEnvironment,-1));
}

// callback for prev env button
void TW_CALL TweakBar::absoluteEnvironment(void* data)
{
	ARDev::PublishEvent(new EnvironmentEventObject(EOE_AbsoluteEnvironment, *reinterpret_cast<int*>(data)));
}

// callback for clear paths button
void TW_CALL TweakBar::clear(void* data)
{
	ARDev::PublishEvent(new EventObject(ROE_Clear));
}

// setup call to register environments in the gui
void TweakBar::addEnvironment(int i, const char* envName)
{
	char args[40];
	char env[10];
	sprintf(env,"env%i",i+1);
	sprintf(args," label='%i | %s' group='Views' ",i+1, envName!=NULL?envName:"Environment");
	// TODO: Minor memory leak of int. Should be fixed within this class.
	TwAddButton(envbar,strdup(env),TweakBar::absoluteEnvironment,new int(i),strdup(args));
	//printf("Adding environment %i: %s\n",*i+1,envName);
}

void TweakBar::removeDefaultEnv()
{
	TwRemoveVar(envbar,"EnvEmpty");
}

// callback for player disconnect button
void TW_CALL TweakBar::PlayerDisconnect(void*)
{
	ARDev::PublishEvent(new EventObject(POE_DisconnectPlayerInterfaces,true));
}

void TW_CALL TweakBar::SetAutoConnectTW(const void *value,void *clientData)
{
	bool on = *static_cast<const bool *>(value);
	if(on)
		ARDev::PublishEvent(new EventObject(POE_AutoOnPlayerInterfaces));
	else
		ARDev::PublishEvent(new EventObject(POE_AutoOffPlayerInterfaces));
}

void TW_CALL TweakBar::GetAutoConnectTW(void *value,void *clientData)
{
#ifdef HAVE_PLAYER
	*static_cast<bool *>(value) = PlayerClientInterface::AutoConnect;
#else
	*static_cast<bool *>(value) = false;
#endif
}

// callback for exit button
void TW_CALL TweakBar::Exit(void*)
{
	exit(0); // How is this a good idea?
}

// callback for setting render enabled
void TW_CALL TweakBar::SetEnabledTW(const void *value,void *clientData)
{
	static_cast<RenderObject *>(clientData)->SetEnabled(*static_cast<const bool *>(value));
}

// callback for getting render enabled settings
void TW_CALL TweakBar::GetEnabledTW(void *value,void *clientData)
{
	*static_cast<bool *>(value) = static_cast<RenderObject *>(clientData)->Enabled();
}

// callback for setting colour
void TW_CALL TweakBar::SetColourTW(const void *value,void *clientData)
{
	const unsigned char *col = static_cast<const unsigned char*>(value);
	ARColour colour = ARColour(col[0]/255.0f,col[1]/255.0f,col[2]/255.0f,col[3]/255.0f);
	static_cast<RenderObject *>(clientData)->SetColour(colour);
}

// callback for getting colour
void TW_CALL TweakBar::GetColourTW(void *value,void *clientData)
{
	ARColour colour = static_cast<RenderObject *>(clientData)->GetColour();
	unsigned char* val = static_cast<unsigned char*>(value);
	val[0] = colour.r*255.0f;
	val[1] =colour.g*255.0f;
	val[2] =colour.b*255.0f;
	val[3] =colour.a*255.0f;
}

void TweakBar::SelectWindow(RenderObject* selected)
{
	if(selected)
	{
		TwRemoveAllVars(selectbar);
		TwDefine(" 'Select' visible=true ");

		//ARPoint OriginMask=selected->GetParent()->Pos->PositionMask.Origin;
		//ARPoint DirectionMask=selected->GetParent()->Pos->PositionMask.Direction;

//		char uid[50];
		char label[100];
//		int i =0;
		//char args[200];

		snprintf(label,100," label='%s' ",selected->GetParent()->Name);
		TwAddButton(selectbar,"bName",NULL,NULL,label);
	}
	else
	{
		TwDefine(" 'Select' visible=false ");
	}
}



#endif //HAVE_ANTTWEAKBAR
