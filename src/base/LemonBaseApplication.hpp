/*
 * SPDX-FileCopyrightText: 2020-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//
#include <SingleApplication>
//
#include "base/LemonApplicationInterface.hpp"

namespace Lemon {
	class LemonBaseApplication : public SingleApplication, public LemonApplicationInterface {
		Q_OBJECT

	  public:
		LemonBaseApplication(int &argc, char *argv[])
		    : SingleApplication(argc, argv, true, User | ExcludeAppPath | ExcludeAppVersion),
		      LemonApplicationInterface(){};
		virtual ~LemonBaseApplication(){};

		virtual bool Initialize() final;

	  private:
		bool parseCommandLine(bool *canContinue, QString *errorMessage);
	};
} // namespace Lemon
