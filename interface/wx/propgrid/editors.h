/////////////////////////////////////////////////////////////////////////////
// Name:        editors.h
// Purpose:     interface of wxPropertyGrid editors
// Author:      wxWidgets team
// RCS-ID:      $Id:
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------

/** @class wxPGEditor

    Base class for custom wxPropertyGrid editors.

    @remarks
    - Names of builtin property editors are: TextCtrl, Choice,
      ComboBox, CheckBox, TextCtrlAndButton, and ChoiceAndButton. Additional editors
      include SpinCtrl and DatePickerCtrl, but using them requires calling
      wxPropertyGrid::RegisterAdditionalEditors() prior use.

    - Pointer to builtin editor is available as wxPGEditor_EditorName
      (eg. wxPGEditor_TextCtrl).

    - To add new editor you need to register it first using static function
      wxPropertyGrid::RegisterEditorClass(), with code like this:
        @code
            wxPGEditor* editorPointer = wxPropertyGrid::RegisterEditorClass(new MyEditorClass(), "MyEditor");
        @endcode
      After that, wxPropertyGrid will take ownership of the given object, but
      you should still store editorPointer somewhere, so you can pass it to
      wxPGProperty::SetEditor(), or return it from wxPGEditor::DoGetEditorClass().

    @library{wxpropgrid}
    @category{propgrid}
*/
class wxPGEditor : public wxObject
{
public:

    /** Constructor. */
    wxPGEditor()
        : wxObject()
    {
        m_clientData = NULL;
    }

    /** Destructor. */
    virtual ~wxPGEditor();

    /** Returns pointer to the name of the editor. For example, wxPG_EDITOR(TextCtrl)
        has name "TextCtrl". This method is autogenerated for custom editors.
    */
    virtual wxString GetName() const = 0;

    /** Instantiates editor controls.
        @param propgrid
        wxPropertyGrid to which the property belongs (use as parent for control).
        @param property
        Property for which this method is called.
        @param pos
        Position, inside wxPropertyGrid, to create control(s) to.
        @param size
        Initial size for control(s).

        @remarks
        - Primary control shall use id wxPG_SUBID1, and secondary (button) control
          shall use wxPG_SUBID2.
        - Implementation shoud use connect all necessary events to the
          wxPropertyGrid::OnCustomEditorEvent. For Example:
            @code
                // Relays wxEVT_COMMAND_TEXT_UPDATED events of primary editor
                // control to the OnEvent.
                // NOTE: This event in particular is actually automatically conveyed, but
                //   it is just used as an example.
                propgrid->Connect( wxPG_SUBID1, wxEVT_COMMAND_TEXT_UPDATED,
                                  wxCommandEventHandler(wxPropertyGrid::OnCustomEditorEvent) );
            @endcode
            OnCustomEditorEvent will then forward events, first to wxPGEditor::OnEvent and then to wxPGProperty::OnEvent.

    */
    virtual wxPGWindowList CreateControls( wxPropertyGrid* propgrid, wxPGProperty* property,
        const wxPoint& pos, const wxSize& size ) const = 0;

    /** Loads value from property to the control. */
    virtual void UpdateControl( wxPGProperty* property, wxWindow* ctrl ) const = 0;

    /** Draws value for given property.
    */
    virtual void DrawValue( wxDC& dc, const wxRect& rect, wxPGProperty* property, const wxString& text ) const;

    /** Handles events. Returns true if value in control was modified
        (see wxPGProperty::OnEvent for more information).
    */
    virtual bool OnEvent( wxPropertyGrid* propgrid, wxPGProperty* property,
        wxWindow* wnd_primary, wxEvent& event ) const = 0;

    /** Returns value from control, via parameter 'variant'.
        Usually ends up calling property's StringToValue or IntToValue.
        Returns true if value was different.
    */
    virtual bool GetValueFromControl( wxVariant& variant, wxPGProperty* property, wxWindow* ctrl ) const;

    /** Sets value in control to unspecified. */
    virtual void SetValueToUnspecified( wxPGProperty* property, wxWindow* ctrl ) const = 0;

    /** Sets control's value specifically from string. */
    virtual void SetControlStringValue( wxPGProperty* property, wxWindow* ctrl, const wxString& txt ) const;

    /** Sets control's value specifically from int (applies to choice etc.). */
    virtual void SetControlIntValue( wxPGProperty* property, wxWindow* ctrl, int value ) const;

    /** Inserts item to existing control. Index -1 means appending.
        Default implementation does nothing. Returns index of item added.
    */
    virtual int InsertItem( wxWindow* ctrl, const wxString& label, int index ) const;

    /** Deletes item from existing control.
        Default implementation does nothing.
    */
    virtual void DeleteItem( wxWindow* ctrl, int index ) const;

    /** Extra processing when control gains focus. For example, wxTextCtrl 
        based controls should select all text.
    */
    virtual void OnFocus( wxPGProperty* property, wxWindow* wnd ) const;

    /** Returns true if control itself can contain the custom image. Default is
        to return false.
    */
    virtual bool CanContainCustomImage() const;

    //
    // This member is public so scripting language bindings
    // wrapper code can access it freely.
    void*       m_clientData;
};

// -----------------------------------------------------------------------

/** @class wxPGMultiButton

    This class can be used to have multiple buttons in a property editor.
    You will need to create a new property editor class, override CreateControls,
    and have it return wxPGMultiButton instance in wxPGWindowList::SetSecondary().
    For instance, here we add three buttons to a textctrl editor:

    @code

    #include <wx/propgrid/editors.h>

    class wxMultiButtonTextCtrlEditor : public wxPGTextCtrlEditor
    {
        WX_PG_DECLARE_EDITOR_CLASS(wxMultiButtonTextCtrlEditor)
    public:
        wxMultiButtonTextCtrlEditor() {}
        virtual ~wxMultiButtonTextCtrlEditor() {}

        wxPG_DECLARE_CREATECONTROLS
        virtual bool OnEvent( wxPropertyGrid* propGrid,
                              wxPGProperty* property,
                              wxWindow* ctrl,
                              wxEvent& event ) const;

    };

    WX_PG_IMPLEMENT_EDITOR_CLASS(MultiButtonTextCtrlEditor, wxMultiButtonTextCtrlEditor,
                                 wxPGTextCtrlEditor)

    wxPGWindowList wxMultiButtonTextCtrlEditor::CreateControls( wxPropertyGrid* propGrid,
                                                                wxPGProperty* property,
                                                                const wxPoint& pos,
                                                                const wxSize& sz ) const
    {
        // Create and populate buttons-subwindow
        wxPGMultiButton* buttons = new wxPGMultiButton( propGrid, sz );

        // Add two regular buttons
        buttons->Add( "..." );
        buttons->Add( "A" );
        // Add a bitmap button
        buttons->Add( wxArtProvider::GetBitmap(wxART_FOLDER) );

        // Create the 'primary' editor control (textctrl in this case)
        wxPGWindowList wndList = wxPGTextCtrlEditor::CreateControls
                                 ( propGrid, property, pos, buttons->GetPrimarySize() );

        // Finally, move buttons-subwindow to correct position and make sure
        // returned wxPGWindowList contains our custom button list.
        buttons->FinalizePosition(pos);

        wndList.SetSecondary( buttons );
        return wndList;
    }

    bool wxMultiButtonTextCtrlEditor::OnEvent( wxPropertyGrid* propGrid,
                                               wxPGProperty* property,
                                               wxWindow* ctrl,
                                               wxEvent& event ) const
    {
        if ( event.GetEventType() == wxEVT_COMMAND_BUTTON_CLICKED )
        {
            wxPGMultiButton* buttons = (wxPGMultiButton*) propGrid->GetEditorControlSecondary();

            if ( event.GetId() == buttons->GetButtonId(0) )
            {
                // Do something when first button is pressed
                return true;
            }
            if ( event.GetId() == buttons->GetButtonId(1) )
            {
                // Do something when first button is pressed
                return true;
            }
            if ( event.GetId() == buttons->GetButtonId(2) )
            {
                // Do something when second button is pressed
                return true;
            }
        }
        return wxPGTextCtrlEditor::OnEvent(propGrid, property, ctrl, event);
    }

    @endcode

    Further to use this editor, code like this can be used:

    @code

        // Register editor class - needs only to be called once
        wxPGRegisterEditorClass( MultiButtonTextCtrlEditor );

        // Insert the property that will have multiple buttons
        propGrid->Append( new wxLongStringProperty("MultipleButtons", wxPG_LABEL) );

        // Change property to use editor created in the previous code segment
        propGrid->SetPropertyEditor( "MultipleButtons", wxPG_EDITOR(MultiButtonTextCtrlEditor) );

    @endcode

    @library{wxpropgrid}
    @category{propgrid}
*/
class WXDLLIMPEXP_PROPGRID wxPGMultiButton : public wxWindow
{
public:

    wxPGMultiButton( wxPropertyGrid* pg, const wxSize& sz );

    virtual ~wxPGMultiButton() { }

    wxWindow* GetButton( unsigned int i ) { return (wxWindow*) m_buttons[i]; }
    const wxWindow* GetButton( unsigned int i ) const { return (const wxWindow*) m_buttons[i]; }

    /** Utility function to be used in event handlers.
    */
    int GetButtonId( unsigned int i ) const { return GetButton(i)->GetId(); }

    /** Returns number of buttons.
    */
    int GetCount() const { return m_buttons.Count(); }

    void Add( const wxString& label, int id = -2 );
    void Add( const wxBitmap& bitmap, int id = -2 );

    wxSize GetPrimarySize() const
    {
        return wxSize(m_fullEditorSize.x - m_buttonsWidth, m_fullEditorSize.y);
    }

    void FinalizePosition( const wxPoint& pos )
    {
        Move( pos.x + m_fullEditorSize.x - m_buttonsWidth, pos.y );
    }
};

// -----------------------------------------------------------------------

#endif // _WX_PROPGRID_EDITORS_H_
