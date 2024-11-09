/* gpgme.js - Javascript integration for gpgme
 * Copyright (C) 2018 Bundesamt für Sicherheit in der Informationstechnik
 *
 * This file is part of GPGME.
 *
 * GPGME is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * GPGME is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <https://www.gnu.org/licenses/>.
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Author(s):
 *     Maximilian Krambach <mkrambach@intevation.de>
 */

/* global describe, it, expect, Gpgmejs, inputvalues */

describe('GPGME context', function (){
    it('Starting a GpgME instance', function (done){
        let prm = Gpgmejs.init({ timeout: 2000 });
        const input = inputvalues.someInputParameter;
        prm.then(
            function (context){
                expect(context).to.be.an('object');
                expect(context.encrypt).to.be.a('function');
                expect(context.decrypt).to.be.a('function');
                expect(context.sign).to.be.a('function');
                expect(context.verify).to.be.a('function');
                context.Keyring = input;
                expect(context.Keyring).to.be.an('object');
                expect(context.Keyring).to.not.equal(input);
                expect(context.Keyring.getKeys).to.be.a('function');
                expect(context.Keyring.getDefaultKey).to.be.a('function');
                expect(context.Keyring.importKey).to.be.a('function');
                expect(context.Keyring.generateKey).to.be.a('function');
                done();
            });
    });

    it('Error message on unsuccessful connection (timeout)', function (done) {
        let prm = Gpgmejs.init({ timeout: 1 });
        prm.then(
            null,
            function (error){
                expect(error).to.be.an('error');
                expect(error.code).to.equal('CONN_TIMEOUT');
                done();
            }
        );
    });
});