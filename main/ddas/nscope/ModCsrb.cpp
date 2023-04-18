#include "ModCsrb.h"
#include "GuiTypes.h"
#include "TGWidget.h"
#include "TG3DLine.h"
#include "config.h"
#include <config_pixie16api.h>
#include <iostream>
#include <bitset>
#include <stdint.h>
using namespace std;

ModCsrb::ModCsrb (const TGWindow * p, const TGWindow * main, int NumModules):
TGTransientFrame (p, main, 10, 10, kHorizontalFrame)
{
  SetCleanup (kDeepCleanup);
  module_number1 = 0;
  char name[20];

  numModules=NumModules;

  mn_vert = new TGVerticalFrame (this, 200, 300);
  mn = new TGHorizontalFrame (mn_vert, 200, 300);
  mn_vert->AddFrame (mn,
		     new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0,
					0));
  AddFrame (mn_vert,
	    new TGLayoutHints (kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
  column1 = new TGVerticalFrame (mn, 200, 300);
  column2 = new TGVerticalFrame (mn, 200, 300);
  column3 = new TGVerticalFrame (mn, 200, 300);
  column4 = new TGVerticalFrame (mn, 200, 300);
  column5 = new TGVerticalFrame (mn, 400, 300);
  column6 = new TGVerticalFrame (mn, 400, 300);
  column7 = new TGVerticalFrame (mn, 400, 300);
  column8 = new TGVerticalFrame (mn, 400, 300);
  column9 = new TGVerticalFrame (mn, 400, 300);
  column10 = new TGVerticalFrame (mn, 400, 300);
  column11 = new TGVerticalFrame (mn, 400, 300);
  column12 = new TGVerticalFrame (mn, 400, 300);
  column13 = new TGVerticalFrame (mn, 400, 300);
  column14 = new TGVerticalFrame (mn, 400, 300);
  column15 = new TGVerticalFrame (mn, 400, 300);
  column16 = new TGVerticalFrame (mn, 400, 300);
  column17 = new TGVerticalFrame (mn, 400, 300);

  buttons = new TGHorizontalFrame (mn_vert, 400, 300);

  mn->AddFrame (column1,
		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column2,
		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column3,
		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column4,
		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column5,
		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column6,
		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column7,
		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column8,
		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column9,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column10,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column11,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column12,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column13,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column14,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column15,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column16,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column17,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));


//////////////////////////first column////////////////////////


  TGTextEntry *te = new TGTextEntry (column1, new TGTextBuffer (100), 10000,
				     te->GetDefaultGC ()(),
				     te->GetDefaultFontStruct (),
				     kRaisedFrame | kDoubleBorder,
				     GetWhitePixel ());

  te->SetText ("Mod #");
  te->Resize (35, 20);
  te->SetEnabled (kFALSE);
  te->SetFrameDrawn (kTRUE);
  column1->AddFrame (te, new TGLayoutHints (kLHintsCenterX, 0, 0, 10, 0));

  TGTextEntry *Labels[16];

  // max 13 modules in one crate
  for (int i = 0; i < 13; i++)
    {
      sprintf (name, "%d", i);
      Labels[i] = new TGTextEntry (column1, new TGTextBuffer (100), 10000,
				   Labels[i]->GetDefaultGC ()(),
				   Labels[i]->GetDefaultFontStruct (),
				   kRaisedFrame | kDoubleBorder,
				   GetWhitePixel ());
      Labels[i]->SetText (name);
      Labels[i]->Resize (35, 20);
      Labels[i]->SetEnabled (kFALSE);
      Labels[i]->SetFrameDrawn (kTRUE);

//      Labels[i] = new TGLabel (column1, name);
      column1->AddFrame (Labels[i],
			 new TGLayoutHints (kLHintsCenterX, 0, 3, 0, 0));
    }

  make_columns (column2, ckBtn, "PUL", "Control pullups",
		2000);
  make_columns (column3, ckBtn_1, "RSV", "reserved",
		3000);
  make_columns (column4, ckBtn_2, "RSV", "reserved", 4000);
  make_columns (column5, ckBtn_3, "RSV", "reserved", 5000);
  make_columns (column6, ckBtn_4, "DIR", "Set module as Director module,", 6000);
  make_columns (column7, ckBtn_5, "RSV", "reserved", 7000);
  make_columns (column8, ckBtn_6, "MAS", "Set module as chassis master", 8000);
  make_columns (column9, ckBtn_7, "GFT", "Select global fast trigger source", 9000);
  make_columns (column10, ckBtn_8, "ET", "Select external trigger source", 9100);
  make_columns (column11, ckBtn_9, "RSV", "reserved", 9200);
  make_columns (column12, ckBtn_10, "INH", "Enable use of external inhibit signal", 9300);
  make_columns (column13, ckBtn_11, "MC", "Distribute clock and triggers to multiple crates", 9400);
  make_columns (column14, ckBtn_12, "SOR", "Sort events by their timestamps", 9500);
  make_columns (column15, ckBtn_13, "TTB", "Distribute fast triggers over the backplane", 9600);
  make_columns (column16, ckBtn_14, "RSV", "reserved", 9700);
  make_columns (column17, ckBtn_15, "RSV", "reserved", 9750);

/////////////////////////////module entry///////////////////////////////

  TGHorizontal3DLine *ln1 = new TGHorizontal3DLine (column1, 50, 2);
  // TGLabel *mod = new TGLabel (buttons, "Module #");

  // numericMod = new TGNumberEntry (buttons, 0, 4, 100, (TGNumberFormat::EStyle) 0, (TGNumberFormat::EAttribute) 1, (TGNumberFormat::ELimit) 3,	// kNELLimitMinMax
  // 				  0, 3);
  // numericMod->SetButtonToNum (0);

  column1->AddFrame (ln1,
		     new TGLayoutHints (kLHintsCenterX | kLHintsCenterY, 0, 0,
					10, 10));
  // buttons->AddFrame (mod, new TGLayoutHints (kLHintsCenterX, 5, 10, 3, 0));
  // buttons->AddFrame (numericMod,
  // 		     new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 20, 0,
  // 					0));

  // numericMod->Associate (this);
  mn_vert->AddFrame (buttons,
		     new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0,
					0));
/////////////////////////////////////////////////////////////////////////
////////////////////////////Buttons/////////////////////////////////////
  LoadButton = new TGTextButton (buttons, "&Load", 4000);
  LoadButton->Associate (this);
  ApplyButton = new TGTextButton (buttons, "&Apply", 4001);
  ApplyButton->Associate (this);
  CancelButton = new TGTextButton (buttons, "&Cancel", 4002);
  CancelButton->Associate (this);
  buttons->AddFrame (LoadButton,
		     new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));
  buttons->AddFrame (ApplyButton,
		     new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));
  buttons->AddFrame (CancelButton,
		     new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));

  MapSubwindows ();
  Resize ();			// resize to default size
  CenterOnParent ();

  SetWindowName ("Module CSRB Register");

  MapWindow ();

}

ModCsrb::~ModCsrb ()
{
}

int
ModCsrb::make_columns (TGVerticalFrame * column, TGCheckButton * ckBtn_g[17],
		       /*char **/std::string title, /*char **/std::string tooltip, int id)
{

  TGTextEntry *ra = new TGTextEntry (column, new TGTextBuffer (100),
				     10000, ra->GetDefaultGC ()(),
				     ra->GetDefaultFontStruct (),
				     kRaisedFrame | kDoubleBorder,
				     GetWhitePixel ());
  ra->SetText (title.c_str());
  ra->Resize (35, 20);
  ra->SetEnabled (kFALSE);
  ra->SetFrameDrawn (kTRUE);
  ra->SetAlignment (kTextCenterX);
  ra->SetToolTipText (tooltip.c_str(), 0);

  column->AddFrame (ra, new TGLayoutHints (kLHintsCenterX, 0, 0, 10, 3));

  // column->AddFrame (ckBtn_g[0] = new TGCheckButton (column, "", id),
  // 		    new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));
  // ckBtn_g[0]->Associate (this);

  //max 13 modules
  for (int i = 0; i < 13; i++)
    {
      column->AddFrame (ckBtn_g[i] =
			new TGCheckButton (column, "", id + i),
			new TGLayoutHints (kLHintsCenterX, 0, 0, 3, 0));
      if(i >= numModules) ckBtn_g[i]->SetState(kButtonDisabled);
      ckBtn_g[i]->Associate (this);
    }
  return 1;
}


/////////////////////////process message//////////////////////////////////

Bool_t
ModCsrb::ProcessMessage (Long_t msg, Long_t parm1, Long_t parm2)
{
  switch (GET_MSG (msg))
    {
    case kC_COMMAND:
      switch (GET_SUBMSG (msg))
	{
	case kCM_BUTTON:
	  switch (parm1)
	    {
	    // case (100):
	    //   if (parm2 == 0)
	    // 	{
	    // 	  if (module_number1 != numModules-1)
	    // 	    {
	    // 	      ++module_number1;
	    // 	      numericMod->SetIntNumber (module_number1);
	    // 	    }
	    // 	}
	    //   else
	    // 	{
	    // 	  if (module_number1 != 0)
	    // 	    {
	    // 	      if (--module_number1 == 0)
	    // 		module_number1 = 0;
	    // 	      numericMod->SetIntNumber (module_number1);
	    // 	    }
	    // 	}
	    //   break;
	    case 4000:
	      {
		Load_Once = true;
		load_info (module_number1);
	      }
	      break;
	    case 4001:
	      if (Load_Once)
		change_values (module_number1);
	      else
		std::cout << "please load once first !\n";
	      break;
	    case 4002:		/// Cancel Button
	      DeleteWindow ();
	      break;
	    default:
	      break;
	    }
	  break;
	default:
	  break;
	}
      break;
    default:
      break;
    }

  return kTRUE;
}
/**
 * We need to be a bit creative with bit manipulations with the APPnn_BITxx
 * functions getting deprecated and Pixie16Read/WriteSglModPar expecting
 * doubles -- if we're not really careful, casts are conversions for
 * numeric values which will corrupt the bits. Thanks a lot XIA.
 */
int
ModCsrb::load_info (Long_t mod)
{
  unsigned int ModParData;
  int retval;
  
  char pMODULE_CSRB[]="MODULE_CSRB";

  for (int i = 0; i < numModules; i++)
    {
      retval =
        Pixie16ReadSglModPar (/*"MODULE_CSRB"*/pMODULE_CSRB, &ModParData, i/*module_number1*/);
      cout << "Channel " << i << " " << std::hex << ModParData << std::dec << endl;
      uint32_t* pModParData = reinterpret_cast<uint32_t*>(&ModParData);
      std::bitset<16> csrb(*pModParData);
      cout << "Bitset: " << std::hex << csrb.to_ulong() << std::dec << std::endl;
      
      
      //////////////// test gt///////////////
      
      if (!csrb.test(0))
        ckBtn[i]->SetState (kButtonUp);
      else
        ckBtn[i]->SetState (kButtonDown);
  
      ///////////////test mil/////////////////////////
      
      if (!csrb.test(1))
        ckBtn_1[i]->SetState (kButtonUp);
      else
        ckBtn_1[i]->SetState (kButtonDown);
  
      ////////////////test gc////////////////////////////
      
      if (!csrb.test(2))
        ckBtn_2[i]->SetState (kButtonUp);
      else
        ckBtn_2[i]->SetState (kButtonDown);
  
      /////////////////test ra///////////////////////////
      
      if (!csrb.test(3))
        ckBtn_3[i]->SetState (kButtonUp);
      else
        ckBtn_3[i]->SetState (kButtonDown);
  
      //////////////////test ea//////////////////////////
      
      if (!csrb.test(4))
        ckBtn_4[i]->SetState (kButtonUp);
      else
        ckBtn_4[i]->SetState (kButtonDown);
  
      //////////////////test ha///////////////////////////
      
      if (!csrb.test(5))
        ckBtn_5[i]->SetState (kButtonUp);
      else
        ckBtn_5[i]->SetState (kButtonDown);

      /////////////////test gt//////////////////////////////
      
      if (!csrb.test(6))
        ckBtn_6[i]->SetState (kButtonUp);
      else
        ckBtn_6[i]->SetState (kButtonDown);
  
      /////////////////test gt//////////////////////////////
      
      if (!csrb.test(7))
        ckBtn_7[i]->SetState (kButtonUp);
      else
        ckBtn_7[i]->SetState (kButtonDown);
      
      /////////////////test gt//////////////////////////////
      
      if (!csrb.test(8))
        ckBtn_8[i]->SetState (kButtonUp);
      else
        ckBtn_8[i]->SetState (kButtonDown);
  
      /////////////////test gt//////////////////////////////
      
      if (!csrb.test(9))
        ckBtn_9[i]->SetState (kButtonUp);
      else
        ckBtn_9[i]->SetState (kButtonDown);
  
      /////////////////test gt//////////////////////////////
      
      if (!csrb.test(10))
        ckBtn_10[i]->SetState (kButtonUp);
      else
        ckBtn_10[i]->SetState (kButtonDown);
  
      /////////////////test gt//////////////////////////////
      
      if (!csrb.test(11))
        ckBtn_11[i]->SetState (kButtonUp);
      else
        ckBtn_11[i]->SetState (kButtonDown);

      /////////////////test gt//////////////////////////////
      
      if (!csrb.test(12))
        ckBtn_12[i]->SetState (kButtonUp);
      else
        ckBtn_12[i]->SetState (kButtonDown);
  
      /////////////////test gt//////////////////////////////
      
      if (!csrb.test(13))
        ckBtn_13[i]->SetState (kButtonUp);
      else
        ckBtn_13[i]->SetState (kButtonDown);
  
      /////////////////test gt//////////////////////////////
  
      if (!csrb.test(14))
        ckBtn_14[i]->SetState (kButtonUp);
      else
        ckBtn_14[i]->SetState (kButtonDown);
  
      /////////////////test gt//////////////////////////////
      
      if (!csrb.test(15))
        ckBtn_15[i]->SetState (kButtonUp);
      else
        ckBtn_15[i]->SetState (kButtonDown);
    }

  //std::cout << "loading info\n";
  return retval;
}

int
ModCsrb::change_values (Long_t mod)
{
  unsigned int ModParData;
  //int retval;
  char pMODULE_CSRB[]="MODULE_CSRB";

  for (int i = 0; i < numModules; i++)
    {
      
      // XIA numbering.
      std::bitset<16> csrb(0);              // now we just have to set bits that are checked:
      if (ckBtn[i]->IsDown())
        csrb.set(0);

      if (ckBtn_1[i]->IsDown())
        csrb.set(1);

      if (ckBtn_2[i] ->IsDown())
        csrb.set(2);
 
     if (ckBtn_3[i] ->IsDown())
        csrb.set(3);
	
     if (ckBtn_4[i] ->IsDown())
        csrb.set(4);

      if (ckBtn_5[i] ->IsDown())
        csrb.set(5);
	
      if (ckBtn_6[i] ->IsDown())
        csrb.set(6);
	
      if (ckBtn_7[i] ->IsDown())
        csrb.set(7);

      if (ckBtn_8[i] ->IsDown())
        csrb.set(8);

      if (ckBtn_9[i] ->IsDown())
        csrb.set(9);
 
      if (ckBtn_10[i] ->IsDown())
        csrb.set(10);
	
      if (ckBtn_11[i] ->IsDown())
        csrb.set(11);

      if (ckBtn_12[i] ->IsDown())
        csrb.set(12);
	
      if (ckBtn_13[i] ->IsDown())
        csrb.set(13);

      if (ckBtn_14[i] ->IsDown())
        csrb.set(14);
	
      if (ckBtn_15[i] ->IsDown())
        csrb.set(15);
      // So there won't be a conversion to double which corrupts the bits:
      
      
      ModParData = csrb.to_ulong();
      std::cout << "Writing CSRB with: " << std::hex << ModParData << std::dec << std::endl;
      Pixie16WriteSglModPar (/*"MODULE_CSRB"*/pMODULE_CSRB, ModParData, i/*module_number1*/);
  }
  
  return 1;
}
