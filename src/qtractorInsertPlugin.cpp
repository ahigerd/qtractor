// qtractorInsertPlugin.cpp
//
/****************************************************************************
   Copyright (C) 2005-2009, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "qtractorAbout.h"

#include "qtractorInsertPlugin.h"

#include "qtractorSession.h"
#include "qtractorAudioEngine.h"


//----------------------------------------------------------------------------
// qtractorInsertPluginType -- Insert pseudo-plugin type instance.
//

// Derived methods.
bool qtractorInsertPluginType::open (void)
{
	// Sanity check...
	if (m_iChannels < 1)
		return false;

#ifdef CONFIG_DEBUG
	qDebug("qtractorInsertPluginType[%p]::open() channels=%u",
		this, m_iChannels);
#endif

	// Pseudo-plugin type names.
	m_sName  = "Insert";
	m_sLabel = m_sName;

	// Pseudo-plugin unique identifier.
	m_iUniqueID = m_iChannels;

	// Pseudo-plugin port counts...
	m_iControlIns  = 0;
	m_iControlOuts = 0;
	m_iAudioIns    = m_iChannels;
	m_iAudioOuts   = m_iChannels;
	m_iMidiIns     = 0;
	m_iMidiOuts    = 0;

	// Cache flags.
	m_bRealtime = true;

	// Done.
	return true;
}


void qtractorInsertPluginType::close (void)
{
}


// Factory method (static)
qtractorInsertPluginType *qtractorInsertPluginType::createType (
	unsigned short iChannels )
{
	// Sanity check...
	if (iChannels < 1)
		return NULL;

	// Yep, most probably its a valid pseu-plugin...
	return new qtractorInsertPluginType(iChannels);
}


//----------------------------------------------------------------------------
// qtractorInsertPlugin -- Insert pseudo-plugin instance.
//

// Constructors.
qtractorInsertPlugin::qtractorInsertPlugin ( qtractorPluginList *pList,
	qtractorInsertPluginType *pInsertType )
	: qtractorPlugin(pList, pInsertType)
{
#ifdef CONFIG_DEBUG
	qDebug("qtractorInsertPlugin[%p] channels=%u",
		this, pInsertType->channels());
#endif

	setChannels(pInsertType->channels());
}


// Destructor.
qtractorInsertPlugin::~qtractorInsertPlugin (void)
{
	// Cleanup all plugin instances...
	setChannels(0);
}


// Channel/instance number accessors.
void qtractorInsertPlugin::setChannels ( unsigned short iChannels )
{
	// Check our type...
	qtractorPluginType *pType = type();
	if (pType == NULL)
		return;

	// We'll need this globals...
	qtractorSession *pSession = qtractorSession::getInstance();
	if (pSession == NULL)
		return;

	qtractorAudioEngine *pAudioEngine = pSession->audioEngine();
	if (pAudioEngine == NULL)
		return;
	
	// Estimate the (new) number of instances...
	unsigned short iInstances
		= pType->instances(iChannels, pType->isMidi());
	// Now see if instance count changed anyhow...
	if (iInstances == instances())
		return;

	// Gotta go for a while...
	bool bActivated = isActivated();
	setActivated(false);

	// TODO: Cleanup bus...
	if (m_pAudioBus) {
	//	pAudioEngine->detach(m_pAudioBus);
		m_pAudioBus->close();
		delete m_pAudioBus;
		m_pAudioBus = NULL;
	}

	// Set new instance number...
	setInstances(iInstances);
	if (iInstances < 1) {
		setActivated(bActivated);
		return;
	}

#ifdef CONFIG_DEBUG
	qDebug("qtractorInsertPlugin[%p]::setChannels(%u) instances=%u",
		this, iChannels, iInstances);
#endif

	// TODO: Create audio bus...
	m_pAudioBus = new qtractorAudioBus(pAudioEngine,
		pType->label(), qtractorBus::Duplex, false, iChannels, false);

//	pAudioEngine->attach(m_pAudioBus);
	m_pAudioBus->open();

	// (Re)activate instance if necessary...
	setActivated(bActivated);
}


// Do the actual activation.
void qtractorInsertPlugin::activate (void)
{
}


// Do the actual deactivation.
void qtractorInsertPlugin::deactivate (void)
{
}


// The main plugin processing procedure.
void qtractorInsertPlugin::process (
	float **ppIBuffer, float **ppOBuffer, unsigned int nframes )
{
	if (m_pAudioBus == NULL)
		return;

	m_pAudioBus->process_prepare(nframes);
	
	float **ppOut = m_pAudioBus->out(); // Sends.
	float **ppIn  = m_pAudioBus->in();  // Returns.

	unsigned short iChannels = channels();
	
	for (unsigned short i = 0; i < iChannels; ++i) {
		::memcpy(ppOut[i], ppIBuffer[i], nframes);
		::memcpy(ppOBuffer[i], ppIn[i], nframes);
	}

//	m_pAudioBus->process_commit(nframes);
}


// end of qtractorInsertPlugin.cpp
