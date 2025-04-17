# 🚀 ft_irc - Internet Relay Chat Server  

**42 Firenze Project | C++98**  
*Un server IRC compatibile con client standard, realizzato in C++98 con gestione non bloccante di più client.*  

---

## 📋 Specifiche  
✅ **Conformità**: Implementa il protocollo IRC base (RFC 1459)  
✅ **Tecnologie**:  
   - `C++98` (senza Boost/external libs)  
   - `poll()`/`select()` per I/O non bloccante  
   - Socket TCP/IP (IPv4/IPv6)

✅ **Funzionalità**:  
   - Autenticazione (NICK/USER/PASS)  
   - Canali pubblici/privati  
   - Comandi operatori (KICK, TOPIC, MODE, INVITE)  
   - Messaggi privati e broadcast  

---

## 🛠 Installazione & Utilizzo  
```bash
make
./ircserv <port> <password>
